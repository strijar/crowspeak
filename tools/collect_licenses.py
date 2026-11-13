#!/usr/bin/env python3
"""Verify the committed CrowPanel notice inventory offline.

Dependency upgrades require reviewing and updating the inventory and notice
texts together. Firmware packaging consumes this bundle without SDK sources.
"""
import argparse
import hashlib
import json
from pathlib import Path
import re
import tomllib
from release_identity import source_pins

ROOT = Path(__file__).resolve().parents[1]


def digest(data):
    return hashlib.sha256(data).hexdigest()


def input_pins():
    return {
        'platformio_sha256': digest((ROOT / 'platformio.ini').read_bytes()),
        'sdk_manifest_sha256': digest((ROOT / 'vendor/crowpanel-sdk/manifest.json').read_bytes()),
        'cargo_lock_sha256': digest((ROOT / 'protocol/Cargo.lock').read_bytes()),
        'lite_commits': source_pins(ROOT, role='firmware'),
    }


def write_bundle(components, output):
    bundle = bytearray(b"Ratspeak Handheld third-party notices\n\n"
        b"Notices for the pinned build dependencies and embedded runtime components.\n"
        b"Some entries are build-time or board-conditional; this is not a binary SBOM.\n"
        b"Each component retains its own license. See manifest.json for provenance.\n")
    seen = {}
    for component in components:
        bundle.extend(("\n" + "=" * 72 + "\n" + component["name"] + " " + component["version"] + "\n"
            + component["source"] + "\nBoards: " + ", ".join(component["boards"])
            + "\nModes: " + ", ".join(component["modes"]) + "\n").encode())
        for notice in component["notices"]:
            data = notice.pop("data")
            bundle.extend(("\n--- " + notice["path"] + " ---\n").encode())
            notice["length"] = len(data)
            notice["sha256"] = digest(data)
            if notice["sha256"] in seen:
                notice["offset"], original = seen[notice["sha256"]]
                bundle.extend(("The same license text is reproduced above under " + original + ".\n").encode())
            else:
                notice["offset"] = len(bundle)
                seen[notice["sha256"]] = (len(bundle), component["name"] + " / " + notice["path"])
                bundle.extend(data)
                bundle.extend(b"\n")
    manifest = {"schema_version": 1, "inputs": input_pins(), "bundle": "THIRD-PARTY.txt",
                "bundle_sha256": digest(bundle), "components": components}
    output.mkdir(parents=True, exist_ok=True)
    (output / "THIRD-PARTY.txt").write_bytes(bundle)
    (output / "manifest.json").write_text(json.dumps(manifest, indent=2, ensure_ascii=False) + "\n")


def check_bundle(output=ROOT / 'licenses', *, check_inputs=True):
    manifest = json.loads((output / 'manifest.json').read_text())
    bundle = (output / 'THIRD-PARTY.txt').read_bytes()
    if manifest.get('schema_version') != 1 or (check_inputs and manifest['inputs'] != input_pins()):
        raise ValueError('Notice inventory differs from build inputs; review dependency notices')
    if digest(bundle) != manifest['bundle_sha256']:
        raise ValueError('Notice bundle checksum mismatch')
    components = manifest['components']
    required = {'Arduino-ESP32', 'ArduinoJson', 'LovyanGFX', 'lvgl',
                'Elecrow ESP32-S3 120 MHz SDK subset', 'Rust core, alloc and compiler-builtins',
                'Montserrat', 'Font Awesome 5.9.0'}
    if not required <= {c['name'] for c in components}:
        raise ValueError('Missing required dependency notices')
    identities = [(c['name'], c['version']) for c in components]
    if len(set(identities)) != len(identities):
        raise ValueError('Duplicate notice component')
    for name, version in manifest['inputs']['lite_commits'].items():
        if (name, version) not in identities:
            raise ValueError('Missing pinned protocol notice: ' + name)
    for c in components:
        if c['boards'] != ['crowpanel'] or c['modes'] != ['standalone'] or not c['notices']:
            raise ValueError('Invalid notice scope: ' + c['name'])
        for n in c['notices']:
            start, length = n['offset'], n['length']
            if start < 0 or length <= 0 or start + length > len(bundle) or digest(bundle[start:start+length]) != n['sha256']:
                raise ValueError('Notice checksum mismatch: ' + c['name'])
            if not n['source'].startswith('https://'):
                raise ValueError('Notice source must use HTTPS')
            # Embedded adaptations must still retain the reviewed notice bytes.
            for source in c.get('source_files', []):
                if bundle[start:start+length].strip() not in (ROOT / source).read_bytes():
                    raise ValueError('Source notice changed: ' + source)
    lock = tomllib.loads((ROOT / 'protocol/Cargo.lock').read_text())
    expected = {(p['name'],p['version'],p['checksum']) for p in lock['package'] if 'source' in p}
    actual = {(c['name'],c['version'],c['package_sha256']) for c in components if 'package_sha256' in c}
    if actual != expected:
        raise ValueError('Missing locked Rust dependency notice')
    if re.search(rb'/(?:Users|home)/[^/\s]+/', bundle):
        raise ValueError('Local path in notice bundle')
    return len(components)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--check', action='store_true')
    parser.add_argument('--output', type=Path, default=ROOT / 'licenses')
    args = parser.parse_args()
    print(f'License inventory: PASS ({check_bundle(args.output)} components)')
