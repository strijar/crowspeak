# CrowPanel Advance 5-inch standalone

Target: Elecrow CrowPanel Advance 5-inch **V1.2 or V1.3**, ESP32-S3-WROOM-1-N16R8,
800×480 ST7262 RGB panel, GT911 touch, and the manufacturer's **SX1262** expansion.
V1.0/V1.1 backlight commands differ and are not supported by this target.

The firmware uses the shared Ratspeak Reticulum/LXMF services and the included
`small` ESP32-S3 Rust library. It is a standalone messenger with internal flash
storage. SD, audio, RNode and the dual-mode launcher are disabled. Wi-Fi remains
available in Settings but is OFF by default. The interface stays English.

## Hardware

Select the wireless expansion function using the board's physical switches.
Do not select SD, speaker or microphone routing: these functions reuse the radio
pins. The firmware does not drive an SD CS or initialize audio.

| SX1262 signal | ESP32 GPIO |
| --- | --- |
| SCK / MISO / MOSI | 5 / 4 / 6 |
| NSS / DIO1 | 8 / 20 |
| RESET / BUSY | 19 / 2 |

TCXO uses **3.3 V** (SX1262 command value `0x07`), matching Elecrow's radio example;
DIO2 controls the RF switch. SPI is 8 MHz. The initial radio profile is 868 MHz,
BW 250 kHz, SF9, CR 4/5, preamble 18, TX power 17 dBm. Set frequency and modulation
to match the other Reticulum radio before checking interoperability.

Display wiring follows the V1.2/V1.3 example: RGB565 with DE42, VSYNC41, HSYNC40,
PCLK39. Pixel clock starts at 16 MHz. I²C uses SDA15/SCL16. GT911 is polled at
`0x5D` (fallback `0x14`); the board controller is at `0x30`. Touch recovery is
bounded and releases GPIO1 after the reset pulse. Brightness uses the inverted
V1.2/V1.3 command range 0–245. Battery percentage is unavailable because no
verified measurement adapter is implemented.

## Tested status

Hardware testing of this port has confirmed:

- Boot, RGB display and touch input.
- Sending and receiving messages through the manufacturer's SX1262 module.
- Wi-Fi operation (reported by the device owner; individual AP/TCP scenarios
  have not been separately qualified here).
- RU/EN touch keyboard and custom radio controls.
- Removal of the display synchronization artifacts observed during language
  changes, after switching RGB output to SRAM bounce buffers.

The larger fonts and Home/chat layouts have also been checked on the device.
Large redraws can still be visible. The retained configuration is two 20-line
RGB bounce buffers and two 40-line LVGL draw buffers; increasing the buffers
improved this only slightly. This is not a tear-free, double-framebuffer renderer.
V1.2 and V1.3 share this target; these reports do not imply separate testing of
both hardware revisions or every application feature.

## Build prerequisites

The commands below are for Linux, run from the root of this firmware checkout
unless stated otherwise. This checkout builds CrowPanel Advance only.

Install Git, Make, **Python 3.12** with `venv`/pip, and a working Internet
connection for the first dependency installation. Arduino CLI, Arduino IDE, an ESP-IDF installation and a Rust
compiler are **not required** for the normal CrowPanel build.

Check Python before creating the environment:

```sh
python3.12 --version
```

If it is installed outside `PATH`, use its full path, for example
`/usr/local/bin/python3.12`. Do not create the environment with another Python
version and assume that activating it will select 3.12.

### Bundled SDK

The required Elecrow SDK subset is included as `vendor/crowpanel-sdk/sdk.tar.gz`.
PlatformIO installs it automatically from this checkout. No manufacturer repository,
Arduino IDE or separate ESP-IDF installation is needed. See
[SDK inventory and comparison](sdk.md) for provenance, reduction and the 3.5-inch comparison.

### 1. Create the build environment

```sh
python3.12 -m venv .venv
.venv/bin/python -m pip install -r requirements-build.txt
.venv/bin/python --version
export PLATFORMIO_CORE_DIR="$PWD/.platformio-core"
.venv/bin/python -m platformio pkg install -e crowpanel
```

The Python requirements pin **PlatformIO 6.1.19** and **esptool 5.2.0**.
The project-local PlatformIO directory isolates the pinned CrowPanel toolchain. Set this
variable again in every new shell before building. There is no need to activate
`.venv` when using the explicit commands above.

PlatformIO downloads the compiler, framework, tools and libraries from the
versions declared in `platformio.ini`:

| Component | Selected version / source |
| --- | --- |
| Platform | [pioarduino 51.03.03 archive](https://github.com/pioarduino/platform-espressif32/releases/download/51.03.03/platform-espressif32.zip) |
| Arduino ESP32 framework | [3.0.2 archive](https://github.com/espressif/arduino-esp32/releases/download/3.0.2/esp32-3.0.2.zip) |
| SDK libraries and 120 MHz bootloader | Bundled Elecrow SDK subset |
| Xtensa ESP32-S3 compiler | GCC 12.2.0, resolved by the platform |
| Graphics | LovyanGFX 1.1.16 and LVGL 8.3.11 |
| JSON | ArduinoJson 7.4.3 |

Use the package installation command rather than manually unpacking archives
into PlatformIO's package directories. The two linked platform/framework
archives alone are not a complete offline toolchain. A first build needs access
to PyPI, GitHub and the PlatformIO package registry. After dependencies are
installed, retain `.venv`, `.platformio-core`, `.pio/libdeps/crowpanel` and the installed SDK package for subsequent builds.

### 2. Build and package

```sh
export PLATFORMIO_CORE_DIR="$PWD/.platformio-core"
make package
.venv/bin/python tools/check_crowpanel_port.py
```

CrowPanel is the default and only build target. The
package target compiles the standalone app, checks its partition size, merges
the selected Elecrow bootloader and partition table, and produces:

| File | Purpose |
| --- | --- |
| `dist/crowpanel-standalone.bin` | Complete image, write at **0x0** |
| `dist/crowpanel-standalone.zip` | Complete image, manifest, checksums and license notices |
| `.pio/build/crowpanel/firmware.bin` | Application only; **not** an image for address 0x0 |
| `build/crowpanel-standalone.bin` | Intermediate copy of the complete image |

The build uses the included `small` ESP32-S3 Rust archive. It does not build
RNode or a launcher. Wi-Fi compatibility fixes are applied to build-local copies
of the reviewed Arduino 3.0.2 sources; installed SDK/framework files are not
patched in place. Do not upgrade the framework independently: the checks reject
unreviewed source versions.

For a clean rebuild of the CrowPanel application:

```sh
.venv/bin/python -m platformio run -e crowpanel -t clean
make package
```

### The 120 MHz distinction

The actual SDK headers and libraries for **qio_opi / octal PSRAM 120 MHz** are
required. A compiler `-D` cannot change a precompiled SDK. `crowpanel_sdk.py`
checks the selected package and memory configuration before compilation.

The build selects **`bootloader_qio_120m.elf`** from `ESP32S3_120M`. As in Arduino's
`qio120` preset, the ROM image header is **DIO / 80 MHz**, while that bootloader
and the SDK configure high-speed memory. Therefore seeing 80 MHz in esptool
`image-info` is expected. Do not substitute the ordinary 80 MHz bootloader or
force `--flash-freq 120m` into an image header. The merge script checks the actual
bootloader source in the build graph before packaging.

The RGB pixel clock and radio SPI clock are independent of PSRAM frequency.
RGB scanout uses the ESP-IDF driver with two internal SRAM bounce buffers of
20 lines each (64 KB total). LVGL uses two separate 40-line draw buffers in
PSRAM (128 KB total) to reduce the number of flushes during large redraws.
LovyanGFX still draws into the PSRAM framebuffer;
DMA reads the SRAM buffers instead of competing directly for PSRAM bandwidth.
The SDK must enable PSRAM instructions/rodata and 64-byte cache lines, checked
at compile time. This targets display shifts during flash writes, including
keyboard language persistence. Validate repeated RU/EN switches and settings
saves on hardware while receiving LoRa messages.

## Touch input

Tap a text field to open the editor. Russian and English layouts, Shift,
numbers/punctuation, cursor movement and UTF-8 backspace are supported. **Done**
applies the draft to the field; **Cancel** leaves it unchanged. Sending a message
still requires the conversation's **SEND** button. Layout changes are persisted
through the normal settings service without reconfiguring the radio.

Settings choices and numeric values have touch editors. The recovery screens
also have touch buttons; destructive recovery requires a separate confirmation.
Custom radio parameters are always available in Settings. Armed settings actions
require a tap, release, then press-and-hold confirmation. Tap the Back header to cancel.

Optional layouts are described in `src/ui/lvgl/KeyboardLayouts.h`. Append layouts
to the registry (EN=0, RU=1 are persistent indices), or supply entries through
`CROW_KEYBOARD_EXTRA_LAYOUTS`. Each layout has three lowercase and uppercase
UTF-8 rows. Add matching glyphs to the font if a language falls outside the
bundled Latin/Cyrillic ranges. This changes input only, not interface language.

## Flash and first setup

Connect the board's USB serial port and list available ports:

```sh
.venv/bin/python -m serial.tools.list_ports
```

Close any serial monitor before flashing. Replace `/dev/ttyUSB0` below with the
actual port (it may instead be `/dev/ttyACM0` or another name):

```sh
.venv/bin/python -m esptool --chip esp32s3 --port /dev/ttyUSB0 --baud 460800 \
  write-flash 0x0 dist/crowpanel-standalone.bin
```

If the connection is unreliable, retry at `--baud 115200`. If automatic reset
cannot enter the bootloader, use the board's BOOT/RESET controls to enter download
mode, then retry. On Linux, a permission error means your account needs access
to the serial device (commonly the `dialout` group).

Use the complete merged image for the first installation so the custom bootloader
is installed too. There is no separate bootloader-flashing step. Routine updates
with the same partition layout do not require `erase-flash`; avoid erasing flash
if you want to retain the stored identity, settings and messages. An installation
over a different firmware/partition layout is not a data migration.

For logs, reopen the port at 115200 baud:

```sh
.venv/bin/python -m platformio device monitor --port /dev/ttyUSB0 --baud 115200
```

After boot, configure your name, matching LoRa parameters, and optionally Wi-Fi
in Settings. Wi-Fi is off by default. Custom LoRa values are available immediately.

## Troubleshooting the build

| Symptom | What to check |
| --- | --- |
| `python3.12` not found | Use the installed Python 3.12 executable's full path; recreate `.venv` if it was made with another Python version. |
| Missing `framework-arduinoespressif32-libs`, SDK headers or bootloader | Run `python tools/check_crowpanel_sdk.py` and reinstall the local package. |
| Wrong Arduino version, PSRAM speed or Wi-Fi source checksum | Use the pinned framework and manufacturer SDK, set `PLATFORMIO_CORE_DIR`, rerun package installation and clean/rebuild. Do not bypass the checks. |
| `EAI_AGAIN`, DNS or download errors | Restore network/DNS access to the package hosts, then rerun the failed pip/PlatformIO install command. |
| Missing compiler, SCons or library package | Rerun `.venv/bin/python -m platformio pkg install -e crowpanel` with the same project-local core directory. |
| `check_crowpanel_port.py` cannot find build outputs | Run `make package` first; the verifier expects `.platformio-core` and the installed bundled SDK. |
| Image header says 80 MHz | Expected; see the bootloader/SDK distinction above. |

## Validation after changes

Host checks:

```sh
.venv/bin/python tools/check_crowpanel_port.py
.venv/bin/python tools/check_runtime_boundary.py
.venv/bin/python tools/release_catalog.py check
```

The basic functions listed under Tested status already work on hardware. For a
new board or after firmware changes, use this regression checklist:

1. Serial at 115200: 8 MiB PSRAM, `[CROW]` configuration, successful display/touch
   initialization, and `[RADIO] LoRa online`.
2. Touch alignment at all four corners, brightness/dimming, and wake without an
   accidental button activation.
3. Set the same radio parameters on the peer. Exchange announces and Russian
   text with another Reticulum/LXMF endpoint; verify delivery acknowledgements.
4. Reboot and confirm identity, contact names, conversation history and keyboard
   language persist.
5. Exercise incoming LoRa traffic while typing/scrolling to check RGB/PSRAM load.

The protocol archive is unchanged. The full upstream Rust provenance/test suite
requires sibling `rsReticulumLite`, `rsLXMFLite` and `rsLXST` source repositories;
these are not needed to build using the included archive.

## Source layout

| Path | Responsibility |
| --- | --- |
| `src/boards/crowpanel/` | Board configuration, RGB bus, display/backlight, touch input and recovery controls |
| `src/core/` | Shared messaging, radio, storage and settings services |
| `src/ui/lvgl/` | Shared screens and CrowPanel layout adjustments |
| `src/ui/lvgl/TouchKeyboard.cpp`, `KeyboardLayouts.h` | Touch editor and language layouts |
| `src/ui/lvgl/fonts/lv_font_crowpanel_*.c` | Generated 14/18/22 px fonts, including Cyrillic |
| `tools/crowpanel_sdk.py`, `crowpanel_merge.py` | SDK validation and complete-image assembly |
| `tools/patch_crowpanel_wifi.py` | Reviewed Arduino 3.0.2 Wi-Fi compatibility fixes |
| `tools/check_crowpanel_port.py` | Packaged image, bootloader and protocol archive verification |

Generated font sources are included, so ordinary firmware builds need no font
tools. To regenerate them, install the system FreeType shared library, install
the CrowPanel dependencies first (they supply `Montserrat-Medium.ttf`), then run:

```sh
.venv/bin/python tools/fonts/crowpanel_fonts.py
```

The generator uses Python's `ctypes`; it does not require Pillow or npm.
`.venv`, `.platformio-core`, `.pio`, `build` and `dist` are local generated
directories, not source files to commit. The SDK archive and its checksum inventory are committed to this repository.
