#!/usr/bin/env python3
"""Create a release, or verify identical existing assets without overwriting them."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import tempfile


def verify_existing(release, assets, download):
    if release.get('draft') or release.get('prerelease'):
        raise RuntimeError('Existing release is not a published stable release')
    remote = {asset['name']: asset for asset in release['assets']}
    if set(remote) != set(assets):
        raise RuntimeError('Existing release asset names differ from this build')
    for name, path in assets.items():
        asset = remote[name]
        expected = 'sha256:' + hashlib.sha256(path.read_bytes()).hexdigest()
        if asset.get('state') != 'uploaded' or asset['size'] != path.stat().st_size:
            raise RuntimeError(f'Existing release asset is incomplete or has a different size: {name}')
        digest = asset.get('digest')
        if not digest:
            digest = 'sha256:' + hashlib.sha256(download(asset)).hexdigest()
        if digest != expected:
            raise RuntimeError(f'Existing release asset checksum differs: {name}')


def publish(tag, directory):
    version = tag.removeprefix('v')
    names = [f'FastScan-Next-V{version}.zip', f'FastScan-V{version}.apk',
             'FastScan-Next-Manual.md', 'SHA256SUMS']
    assets = {name: directory / name for name in names}
    for path in [*assets.values(), directory / 'RELEASE_NOTES.md']:
        if not path.is_file():
            raise RuntimeError(f'Missing release file: {path}')
    result = subprocess.run(['gh', 'api', f'repos/{{owner}}/{{repo}}/releases/tags/{tag}'],
                            capture_output=True, text=True)
    if result.returncode == 0:
        def download(asset):
            with tempfile.TemporaryDirectory() as temporary:
                subprocess.run(['gh', 'release', 'download', tag, '--pattern', asset['name'],
                                '--dir', temporary], check=True)
                return (Path(temporary) / asset['name']).read_bytes()
        verify_existing(json.loads(result.stdout), assets, download)
        print(f'Release {tag} already exists and all assets match; no changes needed.')
        return
    # Do not interpret authentication, rate-limit or network failures as missing releases.
    if 'HTTP 404' not in result.stderr:
        raise RuntimeError('Could not check existing release: ' + result.stderr.strip())
    subprocess.run(['gh', 'release', 'create', tag, '--verify-tag', '--title',
                    f'FastScan Next {version}', '--notes-file', str(directory / 'RELEASE_NOTES.md'),
                    '--latest', *map(str, assets.values())], check=True)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--tag', required=True)
    parser.add_argument('--assets', type=Path, required=True)
    args = parser.parse_args()
    try:
        publish(args.tag, args.assets)
    except (RuntimeError, OSError, ValueError, subprocess.CalledProcessError) as error:
        parser.exit(1, f'Release verification failed: {error}\n')
