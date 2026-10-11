import copy
import hashlib
import importlib.util
import json
import tempfile
import unittest
import zipfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('package_release', ROOT / 'scripts/package_release.py')
release = importlib.util.module_from_spec(spec)
spec.loader.exec_module(release)


class PackageReleaseTests(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name)
        self.output = self.root / 'output'
        content = {
            '扫描软件/FastScan-V3.0.2.apk': b'fixture apk',
            'FastScan-Next-使用手册.md': '当前版本 3.0.2。\n'.encode(),
            '内核驱动/kpm/libkpm.so': b'fixture library',
            '注意事项.txt': b'fixture notes',
        }
        self.manifest = {
            'version': '3.0.2', 'release_date': '2026-10-11',
            'apk': 'FastScan-V3.0.2.apk', 'manual': 'FastScan-Next-使用手册.md',
            'manual_asset': 'FastScan-Next-Manual.md',
            'notes': 'releases/3.0.2.md',
            'source_archive': {'name': 'FastScan-Next-V3.0.2.zip', 'sha256': 'a' * 64},
            'directories': ['内核驱动/', '内核驱动/kpm/', '内核驱动/空目录/'],
            'files': [],
        }
        for name, data in content.items():
            path = self.root / name
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_bytes(data)
            self.manifest['files'].append({
                'source': name, 'archive_path': Path(name).name if name.endswith('.apk') else name,
                'size': len(data), 'sha256': hashlib.sha256(data).hexdigest(),
            })
        (self.root / 'releases').mkdir()
        (self.root / 'releases/3.0.2.md').write_text('# FastScan 3.0.2 更新日志\n', encoding='utf-8')
        self.save()

    def save(self):
        (self.root / 'releases/current.json').write_text(json.dumps(self.manifest, ensure_ascii=False), encoding='utf-8')

    def rejected(self, text):
        self.save()
        with self.assertRaisesRegex(release.ReleaseError, text):
            release.package(self.root, 'v3.0.2', self.output)
        self.assertFalse(self.output.exists(), 'Validation must fail before emitting output')

    def test_packaging_preserves_file_bytes_empty_directories_and_unicode(self):
        (self.root / 'old.apk').write_bytes(b'not released')
        assets = release.package(self.root, 'v3.0.2', self.output)
        self.assertEqual({p.name for p in assets}, {
            'FastScan-Next-V3.0.2.zip', 'FastScan-V3.0.2.apk',
            'FastScan-Next-Manual.md', 'SHA256SUMS', 'RELEASE_NOTES.md',
        })
        with zipfile.ZipFile(self.output / 'FastScan-Next-V3.0.2.zip') as archive:
            self.assertEqual(set(archive.namelist()), set(self.manifest['directories']) | {
                e['archive_path'] for e in self.manifest['files']})
            self.assertIsNone(archive.testzip())
            for entry in self.manifest['files']:
                self.assertEqual(archive.read(entry['archive_path']), (self.root / entry['source']).read_bytes())
        for line in (self.output / 'SHA256SUMS').read_text().splitlines():
            digest, name = line.split('  ', 1)
            self.assertEqual(digest, release.sha256(self.output / name))
        self.assertIn('正式版', (self.output / 'RELEASE_NOTES.md').read_text())

    def test_standalone_manual_name_is_ascii_and_preserves_content(self):
        release.package(self.root, 'v3.0.2', self.output)
        manual = self.output / self.manifest['manual_asset']
        self.assertTrue(manual.name.isascii())
        self.assertEqual(manual.read_bytes(), (self.root / self.manifest['manual']).read_bytes())
        with zipfile.ZipFile(self.output / 'FastScan-Next-V3.0.2.zip') as archive:
            self.assertEqual(archive.read(self.manifest['manual']), manual.read_bytes())
        checksums = (self.output / 'SHA256SUMS').read_text()
        self.assertIn('  ' + manual.name + '\n', checksums)
        self.assertNotIn(self.manifest['manual'], checksums)
        notes = (self.output / 'RELEASE_NOTES.md').read_text()
        self.assertIn(manual.name, notes)

    def test_rejects_unsafe_standalone_manual_name(self):
        for name in ('FastScan-Next-使用手册.md', '../Manual.md', 'other.md'):
            with self.subTest(name=name):
                self.manifest['manual_asset'] = name
                self.rejected('GitHub-safe filename')

    def test_packaging_includes_nested_templates_and_libraries(self):
        content = {
            '动态基址模板/cpp模板/memory.cpp': b'// template source',
            '动态基址模板/cpp模板/lib/arm64-v8a/libSea3.a': b'fixture archive',
            '动态基址模板/lua模板/fscan基址模板.lua': b'-- template source',
        }
        for name, data in content.items():
            path = self.root / name
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_bytes(data)
            self.manifest['files'].append({
                'source': name, 'archive_path': name,
                'size': len(data), 'sha256': hashlib.sha256(data).hexdigest(),
            })
        self.manifest['directories'] += sorted({
            parent.relative_to(self.root).as_posix() + '/'
            for name in content
            for parent in (self.root / name).parents
            if parent != self.root and self.root in parent.parents
        })
        self.save()
        release.package(self.root, 'v3.0.2', self.output)
        with zipfile.ZipFile(self.output / 'FastScan-Next-V3.0.2.zip') as archive:
            for name, data in content.items():
                self.assertEqual(archive.read(name), data)
            for name in self.manifest['directories']:
                self.assertTrue(archive.getinfo(name).is_dir())
        self.assertEqual((self.output / self.manifest['apk']).read_bytes(), b'fixture apk')

    def test_reproducible_bundle(self):
        release.package(self.root, 'v3.0.2', self.output)
        second = self.root / 'second'
        release.package(self.root, 'v3.0.2', second)
        for file in self.output.iterdir():
            self.assertEqual(file.read_bytes(), (second / file.name).read_bytes())

    def test_rejects_wrong_or_untrusted_tag(self):
        for tag in ('main', 'v3.0.1', 'v3.0.2-rc1', 'v3.0.2; echo bad'):
            with self.subTest(tag=tag), self.assertRaises(release.ReleaseError):
                release.package(self.root, tag, self.output)
        self.assertFalse(self.output.exists())

    def test_rejects_changed_file(self):
        (self.root / self.manifest['files'][0]['source']).write_bytes(b'changed apk')
        self.rejected('SHA-256 mismatch')

    def test_rejects_wrong_size(self):
        self.manifest['files'][0]['size'] += 1
        self.rejected('Size mismatch')

    def test_rejects_missing_input(self):
        (self.root / self.manifest['files'][0]['source']).unlink()
        self.rejected('Missing or invalid input')

    def test_rejects_duplicate_member(self):
        self.manifest['files'].append(copy.deepcopy(self.manifest['files'][0]))
        self.rejected('Duplicate release entry')

    def test_rejects_archive_path_traversal(self):
        for path in ('../evil', '/evil', 'folder/../../evil', 'folder\\evil', 'a\nb', 'a//b', 'C:/evil'):
            with self.subTest(path=path), self.assertRaises(release.ReleaseError):
                release.safe_relative(path)

    def test_rejects_source_path_traversal(self):
        self.manifest['files'][0]['source'] = '../evil'
        self.rejected('Unsafe path')

    def test_rejects_symlink_file(self):
        path = self.root / self.manifest['files'][0]['source']
        path.unlink()
        path.symlink_to(self.root / '注意事项.txt')
        self.rejected('Symlinks')

    def test_rejects_symlink_parent(self):
        path = self.root / '扫描软件'
        path.rename(self.root / 'real-directory')
        path.symlink_to(self.root / 'real-directory', target_is_directory=True)
        self.rejected('Symlinks')

    def test_rejects_stale_output(self):
        self.output.mkdir()
        old = self.output / 'old.apk'
        old.write_bytes(b'do not delete')
        with self.assertRaisesRegex(release.ReleaseError, 'empty directory'):
            release.package(self.root, 'v3.0.2', self.output)
        self.assertEqual(old.read_bytes(), b'do not delete')

    def test_rejects_output_symlink(self):
        self.output.symlink_to(self.root / 'elsewhere')
        with self.assertRaisesRegex(release.ReleaseError, 'Output cannot be a symlink'):
            release.package(self.root, 'v3.0.2', self.output)

    def test_rejects_duplicate_directory(self):
        self.manifest['directories'].append('内核驱动/')
        self.rejected('Duplicate archive path')

    def test_rejects_file_directory_collision(self):
        self.manifest['directories'].append('注意事项.txt/child/')
        self.rejected('File used as parent directory')

    def test_rejects_wrong_apk_name(self):
        self.manifest['apk'] = 'FastScan-V3.0.1.apk'
        self.rejected('APK filename')

    def test_rejects_missing_manual(self):
        self.manifest['files'] = [e for e in self.manifest['files'] if e['archive_path'] != self.manifest['manual']]
        self.rejected('manual must be included')

    def test_rejects_additional_apk(self):
        entry = copy.deepcopy(self.manifest['files'][0])
        entry.update(source='old.apk', archive_path='old.apk')
        (self.root / 'old.apk').write_bytes(b'fixture apk')
        self.manifest['files'].append(entry)
        self.rejected('Exactly the current APK')

    def test_rejects_wrong_manual_version(self):
        entry = self.manifest['files'][1]
        data = '当前版本 3.0.1。\n'.encode()
        (self.root / entry['source']).write_bytes(data)
        entry.update(size=len(data), sha256=hashlib.sha256(data).hexdigest())
        self.rejected('Manual version')

    def test_rejects_wrong_notes_version(self):
        (self.root / 'releases/3.0.2.md').write_text('# FastScan 3.0.1 更新日志\n', encoding='utf-8')
        self.rejected('notes heading')

    def test_rejects_invalid_date(self):
        self.manifest['release_date'] = '2026-99-99'
        self.rejected('Invalid release_date')

    def test_rejects_invalid_origin(self):
        self.manifest['source_archive']['sha256'] = 'not a digest'
        self.rejected('provenance')

    def test_repository_distribution_is_valid(self):
        current = json.loads((ROOT / 'releases/current.json').read_text(encoding='utf-8'))
        manifest = release.validate(ROOT, 'v' + current['version'])
        self.assertTrue(manifest['files'])


    def test_repository_manifest_includes_entire_template_directory(self):
        current = json.loads((ROOT / 'releases/current.json').read_text(encoding='utf-8'))
        templates = ROOT / '动态基址模板'
        self.assertTrue(templates.is_dir())
        files = {p.relative_to(ROOT).as_posix() for p in templates.rglob('*') if p.is_file()}
        self.assertTrue(files)
        entries = {
            e['archive_path']: e for e in current['files']
            if e['archive_path'].startswith('动态基址模板/')
        }
        self.assertEqual(set(entries), files)
        for name, entry in entries.items():
            with self.subTest(path=name):
                self.assertEqual(entry['source'], name)
                data = (ROOT / name).read_bytes()
                self.assertEqual(entry['size'], len(data))
                self.assertEqual(entry['sha256'], hashlib.sha256(data).hexdigest())
        directories = {
            p.relative_to(ROOT).as_posix() + '/'
            for p in [templates, *templates.rglob('*')] if p.is_dir()
        }
        self.assertEqual({d for d in current['directories'] if d.startswith('动态基址模板/')}, directories)
        self.assertEqual(set(current['source_archive']['added_files']), files)
        self.assertIn('动态基址模板', release.release_notes(ROOT, current))

    def test_repository_removed_obsolete_tools_and_tutorial(self):
        for name in ('lua集成C扫', '电脑对比', '使用教程.txt'):
            with self.subTest(path=name):
                self.assertFalse((ROOT / name).exists())
                self.assertNotIn(name, (ROOT / 'README.md').read_text(encoding='utf-8'))

    def test_readme_is_user_facing_not_release_maintenance(self):
        readme = (ROOT / 'README.md').read_text(encoding='utf-8')
        for internal in (
            '维护与发布', '仓库结构', 'releases/current.json',
            'scripts/package_release.py', '.github/workflows/',
            'source_archive', 'unittest', 'Run workflow', '推送对应版本标签',
        ):
            with self.subTest(internal=internal):
                self.assertNotIn(internal, readme)
        for section in ('## 下载与安装', '## 快速开始', '## 完整包内容', '## 问题反馈'):
            self.assertIn(section, readme)
        maintenance = (ROOT / 'MAINTAINING.md').read_text(encoding='utf-8')
        self.assertIn('releases/current.json', maintenance)
        self.assertIn('scripts/package_release.py', maintenance)
        current = json.loads((ROOT / 'releases/current.json').read_text(encoding='utf-8'))
        self.assertNotIn('MAINTAINING.md', {e['archive_path'] for e in current['files']})

    def test_readme_links_to_separate_version_history(self):
        readme = (ROOT / 'README.md').read_text(encoding='utf-8')
        self.assertIn('[版本更新](releases/)', readme)
        self.assertNotIn('更新重点', readme)
        self.assertNotIn('修复未获取存储权限时', readme)
        current = json.loads((ROOT / 'releases/current.json').read_text(encoding='utf-8'))
        version = current['version']
        self.assertNotIn(f'## {version}', readme)
        index = (ROOT / 'releases/README.md').read_text(encoding='utf-8')
        self.assertIn(f'({version}.md)', index)
        self.assertTrue((ROOT / current['notes']).is_file())

    def test_repository_v3_documentation_is_consistent(self):
        current = json.loads((ROOT / 'releases/current.json').read_text(encoding='utf-8'))
        readme = (ROOT / 'README.md').read_text(encoding='utf-8')
        notice = (ROOT / '注意事项.txt').read_text(encoding='utf-8')
        self.assertTrue(readme.startswith('# FastScan Next V3\n'))
        self.assertIn(f"当前正式版：**{current['version']}**", readme)
        self.assertIn(f"当前版本：{current['version']} 正式版", notice)
        self.assertIn('FastScan-Next-使用手册.md', notice)
        self.assertNotIn('预发布', notice)
        self.assertNotIn('原包注意事项', release.release_notes(ROOT, current))
        self.assertIn('注意事项.txt', current['source_archive']['modified_files'])


if __name__ == '__main__':
    unittest.main()
