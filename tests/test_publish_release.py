import hashlib
import importlib.util
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch
import subprocess

spec = importlib.util.spec_from_file_location('publish_release', Path(__file__).resolve().parents[1] / 'scripts/publish_release.py')
publisher = importlib.util.module_from_spec(spec)
spec.loader.exec_module(publisher)


class PublishReleaseTests(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.path = Path(self.temporary.name) / 'fixture.apk'
        self.path.write_bytes(b'release')
        self.assets = {self.path.name: self.path}
        self.release = {'draft': False, 'prerelease': False, 'assets': [{
            'name': self.path.name, 'size': 7, 'state': 'uploaded',
            'digest': 'sha256:' + hashlib.sha256(b'release').hexdigest()}]}

    def test_matching_published_assets_need_no_download(self):
        publisher.verify_existing(self.release, self.assets, lambda _: self.fail('unexpected download'))

    def test_older_assets_without_digest_are_downloaded_and_hashed(self):
        self.release['assets'][0]['digest'] = None
        publisher.verify_existing(self.release, self.assets, lambda _: b'release')
        with self.assertRaises(RuntimeError):
            publisher.verify_existing(self.release, self.assets, lambda _: b'changed')

    def test_mismatch_incomplete_missing_extra_and_unpublished_are_rejected(self):
        import copy
        for change in ('digest', 'size', 'state', 'missing', 'extra', 'draft', 'prerelease'):
            candidate = copy.deepcopy(self.release)
            if change == 'digest': candidate['assets'][0]['digest'] = 'sha256:wrong'
            elif change == 'size': candidate['assets'][0]['size'] = 8
            elif change == 'state': candidate['assets'][0]['state'] = 'new'
            elif change == 'missing': candidate['assets'] = []
            elif change == 'extra': candidate['assets'].append({'name': 'extra'})
            else: candidate[change] = True
            with self.subTest(change=change), self.assertRaises(RuntimeError):
                publisher.verify_existing(candidate, self.assets, lambda _: b'release')

    def test_network_and_auth_errors_never_attempt_creation(self):
        directory = Path(self.temporary.name)
        for name in ['FastScan-Next-V3.0.3.zip', 'FastScan-V3.0.3.apk', 'FastScan-Next-Manual.md', 'SHA256SUMS', 'RELEASE_NOTES.md']:
            (directory / name).write_bytes(b'fixture')
        for message in ['HTTP 401', 'HTTP 403', 'connection refused']:
            with patch.object(publisher.subprocess, 'run', return_value=subprocess.CompletedProcess([], 1, '', message)) as run:
                with self.assertRaises(RuntimeError): publisher.publish('v3.0.3', directory)
                self.assertEqual(1, run.call_count)

    def test_only_a_missing_release_creates_a_new_release_with_verified_tag(self):
        directory = Path(self.temporary.name)
        for name in ['FastScan-Next-V3.0.3.zip', 'FastScan-V3.0.3.apk', 'FastScan-Next-Manual.md', 'SHA256SUMS', 'RELEASE_NOTES.md']:
            (directory / name).write_bytes(b'fixture')
        with patch.object(publisher.subprocess, 'run', side_effect=[
            subprocess.CompletedProcess([], 1, '', 'gh: Not Found (HTTP 404)'),
            subprocess.CompletedProcess([], 0),
        ]) as run:
            publisher.publish('v3.0.3', directory)
            self.assertEqual(2, run.call_count)
            command = run.call_args.args[0]
            self.assertEqual(['gh', 'release', 'create', 'v3.0.3'], command[:4])
            self.assertIn('--verify-tag', command)
            self.assertNotIn('--clobber', command)
