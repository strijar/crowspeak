# Bundled CrowPanel SDK

Normal builds install `vendor/crowpanel-sdk/sdk.tar.gz` directly through PlatformIO.
There is no dependency on an external Elecrow checkout. The Arduino 3.0.2 framework,
compiler and graphics libraries are still downloaded on first installation.

## Provenance and selection

Source: Elecrow CrowPanel Advance 5 repository, commit
`b2a9e1f5dad2281974c98bba71214ece5c07b706`, directory `ESP32S3_120M`, package
`5.1.4+sha.bd2b9390ef`. The original SDK contains approximately **218 MiB** of file
contents (227 MiB filesystem allocation). The bundled subset is approximately
**79.6 MiB unpacked / 19.9 MiB compressed**, with 2,535 files.

Retained inputs:

- ESP32-S3 **qio_opi**, including its sdkconfig, linker scripts and memory libraries.
- **bootloader_qio_120m.elf**. The ROM header remains DIO/80 MHz, while the bootloader
  and SDK initialize high-speed memory, matching the existing working firmware.
- SDK archives that contribute object files to the known-good CrowPanel firmware
  linker map, preserving their full contents, debug information and link order.
- Common ESP32-S3 headers for compiling the Arduino core and libraries, including
  transitive and conditional includes, rather than pruning individual headers
  based on one compiler invocation.
- Package metadata, build flags and the PlatformIO builder.

Removed: other memory configurations and bootloaders, unused static libraries
(including large AI, face detection and speech models), runtime speech assets and
unused DSP headers. The builder and `flags/ld_libs` are filtered to reference only
retained SDK libraries and compiler runtime libraries. No binary has been patched
or stripped. Adding new SDK-dependent features may require extending the subset;
missing libraries should fail at link time.

`manifest.json` records the original file hashes, bundled file hashes and compressed
archive hash. The build validates the installed package against this inventory.
`tools/vendor_crowpanel_sdk.py` produces a deterministic archive; ordinary builds
do not run the importer.

```sh
python tools/check_crowpanel_sdk.py
python tools/vendor_crowpanel_sdk.py --sdk /path/to/ESP32S3_120M \
  --map /path/to/known-good/firmware.map
```

After changing the subset, reinstall the local package, clean/rebuild, review
notices and verify the merged image. Keep the importer input map from a successful
full-SDK build when expanding the required library set.

## CrowPanel Advance 3.5 comparison

The supplied 3.5-inch and 5-inch SDK directories each contain 2,727 files with
identical relative paths. **All binary libraries, bootloaders, memory configuration,
linker scripts, package metadata and builder files match byte for byte.**

Seven files differ, all under `esp32s3/include/espressif__esp-dsp/modules/windows/`:
`blackman`, `blackman_harris`, `blackman_nuttall`, `flat_top`, `hann`, `nuttall`, and
the common `include/dsps_wind.h`. In the supplied 3.5-inch SDK these files contain
binary data rather than ordinary C headers. They are unused here and excluded.
Every retained original file matches the 3.5-inch SDK:

```sh
python tools/check_crowpanel_sdk.py --compare /path/to/3.5/ESP32S3_120M
```

Thus both models can share this SDK package. This does **not** make the 5-inch
firmware a 3.5-inch build: panel controller, resolution, timing, pin assignment,
touch coordinates and revision-specific board controls must be implemented and
verified separately. There is deliberately no untested 3.5-inch build environment.

## Validation

A complete CrowPanel build and factory-image packaging were verified using this
local SDK package. Image checks regenerate the bootloader from the bundled ELF,
verify SDK Wi-Fi adapters and Rust archive provenance, compare the partition
layout, and verify image identities and checksums. Hardware behavior still needs
regression testing after replacing the firmware.
