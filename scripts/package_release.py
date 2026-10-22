#!/usr/bin/env python3
"""Validate a distribution manifest and build only its explicitly listed assets."""
from __future__ import annotations

import argparse
import datetime as dt
import hashlib
import json
import re
import shutil
import stat
import tempfile
import zipfile
from pathlib import Path, PurePosixPath


class ReleaseError(ValueError):
    """An invalid release must not be uploaded."""


def safe_relative(value: str) -> PurePosixPath:
    if not isinstance(value, str) or not value or any(c in value for c in '\\\n\r\0'):
        raise ReleaseError(f'Invalid path: {value!r}')
    path = PurePosixPath(value)
    if path.is_absolute() or any(p in ('', '.', '..') for p in value.split('/')) or ':' in value:
        raise ReleaseError(f'Unsafe path: {value!r}')
    return path


def source_file(root: Path, name: str) -> Path:
    relative = safe_relative(name)
    path = root
    for part in relative.parts:
        path /= part
        if path.is_symlink():
            raise ReleaseError(f'Symlinks are not release inputs: {name}')
    if not path.is_file() or not path.resolve().is_relative_to(root):
        raise ReleaseError(f'Missing or invalid input: {name}')
    return path


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open('rb') as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b''):
            digest.update(chunk)
    return digest.hexdigest()


def validate(root: Path, tag: str) -> dict:
    manifest = json.loads(source_file(root, 'releases/current.json').read_text(encoding='utf-8'))
    version = manifest['version']
    if not isinstance(version, str) or not re.fullmatch(r'\d+\.\d+\.\d+', version):
        raise ReleaseError('Only stable x.y.z versions are supported')
    if tag != f'v{version}':
        raise ReleaseError(f'Tag {tag!r} must equal v{version}')
    try:
        release_date = dt.date.fromisoformat(manifest['release_date'])
    except (ValueError, TypeError) as error:
        raise ReleaseError('Invalid release_date') from error
    if release_date.year < 1980:
        raise ReleaseError('release_date must be representable in ZIP files')
    if manifest['apk'] != f'FastScan-V{version}.apk':
        raise ReleaseError('APK filename must match the release version')
    safe_relative(manifest['manual'])
    if manifest.get('manual_asset') != 'FastScan-Next-Manual.md':
        raise ReleaseError('Standalone manual must use the GitHub-safe filename FastScan-Next-Manual.md')
    if manifest['notes'] != f'releases/{version}.md':
        raise ReleaseError('Release notes must match the release version')
    provenance = manifest['source_archive']
    if provenance['name'] != f'FastScan-Next-V{version}.zip' or not re.fullmatch(r'[a-f0-9]{64}', provenance['sha256']):
        raise ReleaseError('Invalid source archive provenance')
    files = manifest['files']
    if not files:
        raise ReleaseError('Release file list is empty')
    sources, names = set(), set()
    for entry in files:
        name = str(safe_relative(entry['archive_path']))
        if name.casefold() in names or entry['source'] in sources:
            raise ReleaseError(f'Duplicate release entry: {name}')
        names.add(name.casefold())
        sources.add(entry['source'])
        path = source_file(root, entry['source'])
        if type(entry['size']) is not int or entry['size'] < 0 or path.stat().st_size != entry['size']:
            raise ReleaseError(f'Size mismatch: {entry["source"]}')
        if not re.fullmatch(r'[a-f0-9]{64}', entry['sha256']) or sha256(path) != entry['sha256']:
            raise ReleaseError(f'SHA-256 mismatch: {entry["source"]}')
    expected_apks = {manifest['apk'].casefold()}
    if manifest.get('test_apk'):
        expected_apks.add(str(safe_relative(manifest['test_apk'])).casefold())
    if {name for name in names if name.endswith('.apk')} != expected_apks:
        raise ReleaseError('Exactly the current APK must be included')
    if manifest['manual'].casefold() not in names:
        raise ReleaseError('The current manual must be included')
    directories = set()
    for name in manifest['directories']:
        if not isinstance(name, str) or not name.endswith('/'):
            raise ReleaseError(f'Invalid directory entry: {name!r}')
        normal = str(safe_relative(name[:-1])).casefold()
        if normal in directories or normal in names:
            raise ReleaseError(f'Duplicate archive path: {name}')
        directories.add(normal)
    for name in names | directories:
        if any(str(parent) in names for parent in PurePosixPath(name).parents if str(parent) != '.'):
            raise ReleaseError(f'File used as parent directory: {name}')
    notes = source_file(root, manifest['notes']).read_text(encoding='utf-8')
    if not notes.startswith(f'# FastScan {version} '):
        raise ReleaseError('Release notes heading does not match version')
    manual_entry = next(e for e in files if e['archive_path'] == manifest['manual'])
    if f'当前版本 {version}。' not in source_file(root, manual_entry['source']).read_text(encoding='utf-8')[:2000]:
        raise ReleaseError('Manual version does not match release')
    return manifest


def release_notes(root: Path, manifest: dict) -> str:
    version = manifest['version']
    return f'''# FastScan Next {version}

## 下载

- **FastScan-Next-V{version}.zip**：完整包，包含 APK、离线手册、配套驱动、动态基址模板及 V3 使用注意事项。
- **{manifest['apk']}**：单独的 Android 安装包。
- **{manifest['manual_asset']}**：离线使用手册，包含 MCP 连接与权限说明。
- **SHA256SUMS**：上述下载文件的 SHA-256 校验值。

这是正式版发布。GitHub 自动附带的 Source code 压缩包只是本分发仓库的快照，不是应用安装包，也不包含完整 Android/核心源码。

## 使用提醒

安装 APK 后配置账号、权限、目标进程及读取器。MCP 已内置在应用中，无需额外安装服务程序。
配套内核驱动不保证全机型适配，跨进程访问需要相应权限。只在你有权操作的设备和应用上使用。
完整包附带 C++ / Lua 动态基址模板及原有配套静态库；历史模板尚未验证与 V3 当前结果格式完全兼容，使用前请核对并按需适配。
安装与使用提醒已更新为 V3 正式版说明，详细流程见同包使用手册。

'''+source_file(root, manifest['notes']).read_text(encoding='utf-8')


def package(root: Path, tag: str, output: Path) -> list[Path]:
    root = root.resolve()
    manifest = validate(root, tag)
    if output.is_symlink():
        raise ReleaseError('Output cannot be a symlink')
    if output.exists() and (not output.is_dir() or any(output.iterdir())):
        raise ReleaseError(f'Output must be an empty directory: {output}')
    output.mkdir(parents=True, exist_ok=True)
    version = manifest['version']
    date = dt.date.fromisoformat(manifest['release_date'])
    stamp = (date.year, date.month, date.day, 0, 0, 0)
    # Stage every output so a failed write never leaves partial release assets.
    with tempfile.TemporaryDirectory(prefix='release-', dir=output.parent) as directory:
        stage = Path(directory)
        bundle = stage / f'FastScan-Next-V{version}.zip'
        with zipfile.ZipFile(bundle, 'w', compression=zipfile.ZIP_DEFLATED, compresslevel=9) as archive:
            for name in sorted(manifest['directories']):
                info = zipfile.ZipInfo(name, stamp)
                info.create_system = 3
                info.external_attr = (stat.S_IFDIR | 0o755) << 16 | 0x10
                archive.writestr(info, b'')
            for entry in sorted(manifest['files'], key=lambda item: item['archive_path']):
                info = zipfile.ZipInfo(entry['archive_path'], stamp)
                info.create_system = 3
                info.external_attr = (stat.S_IFREG | 0o644) << 16
                info.compress_type = zipfile.ZIP_DEFLATED
                with source_file(root, entry['source']).open('rb') as source, archive.open(info, 'w') as target:
                    shutil.copyfileobj(source, target)
        with zipfile.ZipFile(bundle) as archive:
            if archive.testzip() is not None:
                raise ReleaseError('ZIP integrity check failed')
            for entry in manifest['files']:
                if hashlib.sha256(archive.read(entry['archive_path'])).hexdigest() != entry['sha256']:
                    raise ReleaseError(f'Packaged file mismatch: {entry["archive_path"]}')
        for name in (manifest['apk'], manifest['manual']):
            entry = next(e for e in manifest['files'] if e['archive_path'] == name)
            # GitHub rewrites non-ASCII release asset names. Keep the Chinese
            # name inside the ZIP and use a stable ASCII name for direct downloads.
            asset_name = manifest['manual_asset'] if name == manifest['manual'] else name
            shutil.copyfile(source_file(root, entry['source']), stage / asset_name)
            if sha256(stage / asset_name) != entry['sha256']:
                raise ReleaseError(f'Standalone asset mismatch: {asset_name}')
        assets = sorted(stage.iterdir())
        (stage / 'SHA256SUMS').write_text(''.join(f'{sha256(p)}  {p.name}\n' for p in assets), encoding='utf-8')
        (stage / 'RELEASE_NOTES.md').write_text(release_notes(root, manifest), encoding='utf-8')
        for file in stage.iterdir():
            file.replace(output / file.name)
    return sorted(output.iterdir())


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--tag', required=True, help='Existing stable version tag, e.g. v3.0.2')
    parser.add_argument('--root', type=Path, default=Path(__file__).resolve().parents[1])
    parser.add_argument('--output', type=Path, help='Empty output directory (default: ROOT/build/release)')
    args = parser.parse_args()
    try:
        assets = package(args.root, args.tag, args.output or args.root / 'build/release')
    except (ReleaseError, OSError, KeyError, TypeError, StopIteration, zipfile.BadZipFile) as error:
        parser.exit(1, f'Release validation failed: {error}\n')
    for asset in assets:
        print(f'{asset.name}: {asset.stat().st_size} bytes')


if __name__ == '__main__':
    main()
