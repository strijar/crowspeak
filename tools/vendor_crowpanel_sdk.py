#!/usr/bin/env python3
"""Reproduce the bundled SDK from Elecrow's SDK and a known-good linker map.

Normal firmware builds use the committed archive and do not run this tool.
"""
import argparse
import gzip
import hashlib
import io
import json
from pathlib import Path
import re
import tarfile


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--sdk', type=Path, required=True)
    parser.add_argument('--map', type=Path, required=True)
    parser.add_argument('--output', type=Path, default=Path('vendor/crowpanel-sdk'))
    args = parser.parse_args()
    used = args.map.read_text().split('Allocating common symbols')[0]
    libraries = set(re.findall(r'([^/\s]+\.a)\(', used))
    files = {}
    for path in sorted(args.sdk.rglob('*')):
        if not path.is_file():
            continue
        name = path.relative_to(args.sdk).as_posix()
        keep = (name == 'package.json' or name == 'esp32s3/platformio-build.py'
                or name.startswith(('esp32s3/include/', 'esp32s3/ld/', 'esp32s3/flags/', 'esp32s3/qio_opi/'))
                or name == 'esp32s3/bin/bootloader_qio_120m.elf'
                or (name.startswith('esp32s3/lib/') and path.name in libraries))
        # DSP/AI headers are unused by this firmware and differ between vendor repos.
        if name.startswith('esp32s3/include/espressif__esp-dsp/'):
            keep = False
        if path.suffix == '.a' and path.name not in libraries:
            keep = False
        if keep:
            files[name] = path.read_bytes()
    source_hashes = {n: hashlib.sha256(d).hexdigest() for n,d in files.items()}
    retained = {Path(n).stem[3:] for n in files if n.endswith('.a')}
    # Preserve the vendor's link order and repetitions; remove only archives
    # that contributed no objects to the known-good firmware. Toolchain libs stay.
    allowed = retained | {'c', 'm', 'stdc++', 'gcc'}
    builder = files['esp32s3/platformio-build.py'].decode()
    builder = re.sub(r'"-l([^\"]+)"\s*,?', lambda m: m[0] if m[1] in allowed else '', builder)
    files['esp32s3/platformio-build.py'] = builder.encode()
    files['esp32s3/flags/ld_libs'] = (' '.join(t for t in files['esp32s3/flags/ld_libs'].decode().split() if not t.startswith('-l') or t[2:] in allowed)+'\n').encode()
    args.output.mkdir(parents=True, exist_ok=True)
    out = io.BytesIO()
    with gzip.GzipFile(fileobj=out, mode='wb', filename='', mtime=0) as gz:
        with tarfile.open(fileobj=gz, mode='w') as tar:
            for name,data in sorted(files.items()):
                info=tarfile.TarInfo(name); info.size=len(data); info.mode=0o644
                tar.addfile(info,io.BytesIO(data))
    archive=out.getvalue()
    (args.output/'sdk.tar.gz').write_bytes(archive)
    manifest={'source_repository':'https://github.com/Elecrow-RD/CrowPanel-Advance-5-HMI-ESP32-S3-AI-Powered-IPS-Touch-Screen-800x480',
        'source_commit':'b2a9e1f5dad2281974c98bba71214ece5c07b706',
        'source_directory':'ESP32S3_120M', 'version':'5.1.4+sha.bd2b9390ef',
        'archive_sha256':hashlib.sha256(archive).hexdigest(), 'unpacked_bytes':sum(map(len,files.values())),
        'source_files':dict(sorted(source_hashes.items())),
        'files':{n:hashlib.sha256(d).hexdigest() for n,d in sorted(files.items())}}
    (args.output/'manifest.json').write_text(json.dumps(manifest,indent=2)+'\n')
    print(f'{len(files)} files; {len(archive)/2**20:.1f} MiB compressed; {manifest["unpacked_bytes"]/2**20:.1f} MiB unpacked')

if __name__=='__main__':
    main()
