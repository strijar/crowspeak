#!/usr/bin/env python3
"""Verify the bundled SDK archive or an installed package, without vendor repos."""
import hashlib
import json
from pathlib import Path
import tarfile
import argparse

ROOT = Path(__file__).resolve().parents[1]
BUNDLE = ROOT / 'vendor/crowpanel-sdk'


def verify(installed=None):
    manifest = json.loads((BUNDLE / 'manifest.json').read_text())
    if installed is None:
        archive = BUNDLE / 'sdk.tar.gz'
        if hashlib.sha256(archive.read_bytes()).hexdigest() != manifest['archive_sha256']:
            raise ValueError('Bundled SDK archive checksum mismatch')
        with tarfile.open(archive) as tar:
            actual = {m.name: hashlib.sha256(tar.extractfile(m).read()).hexdigest()
                      for m in tar if m.isfile()}
        if actual != manifest['files']:
            raise ValueError('Bundled SDK inventory mismatch')
    else:
        for name, expected in manifest['files'].items():
            if hashlib.sha256((Path(installed) / name).read_bytes()).hexdigest() != expected:
                raise ValueError(f'Installed CrowPanel SDK differs: {name}')
    return manifest


if __name__ == '__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--installed', type=Path)
    parser.add_argument('--compare', type=Path, help='Compare original retained files with another Elecrow SDK')
    args=parser.parse_args()
    manifest=verify(args.installed)
    if args.compare:
        for name, expected in manifest['source_files'].items():
            if hashlib.sha256((args.compare / name).read_bytes()).hexdigest() != expected:
                raise ValueError(f'Elecrow SDK differs: {name}')
        print('All retained original SDK files match the comparison SDK')
    print(f'CrowPanel SDK: PASS ({len(manifest["files"])} files)')
