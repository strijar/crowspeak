# Crowspeak — CrowPanel Reticulum/LXMF messenger

Firmware for **Elecrow CrowPanel Advance 5-inch V1.2/V1.3**, with the SX1262
expansion module, RGB display and RU/EN touch keyboard. Storage uses internal
flash; Wi-Fi is available and disabled by default.

This project supports CrowPanel only. CrowPanel Advance 3.5-inch is planned;
its SDK is compatible with the bundled SDK, but its display and peripheral
configuration are not implemented or tested yet.

## Build

Install Make and Python 3.12, then run:

```sh
python3.12 -m venv .venv
.venv/bin/python -m pip install -r requirements-build.txt
export PLATFORMIO_CORE_DIR="$PWD/.platformio-core"
make package
```

Make automatically uses `.venv/bin/python` and the project-local `.platformio-core`.
Explicit `PYTHON` and `PLATFORMIO_CORE_DIR` overrides remain supported.

The first build downloads the pinned compiler, Arduino framework and libraries.
The required Elecrow SDK subset and Rust protocol archive are already included;
no sibling SDK checkout, Arduino IDE or Rust compiler is required.

Outputs: `dist/crowpanel-standalone.bin` (flash at **0x0**) and
`dist/crowpanel-standalone.zip` (image, checksums and notices).

```sh
make flash PORT=/dev/ttyUSB0
```

See [CrowPanel hardware, setup and testing](docs/crowpanel.md) and
[SDK inventory and comparison](docs/sdk.md).

## Development

```sh
make source-check
.venv/bin/python tools/check_crowpanel_port.py
```

The image check requires a completed `make package`. Hardware checks cover
boot, touch alignment, redraws during flash writes and LoRa message exchange.
Protocol source development additionally requires the pinned sibling Rust
repositories listed in `tools/release_identity.json`; normal firmware builds
use `protocol/prebuilt/xtensa-esp32s3/small/libratspeak_protocol.a`.

Derived from Ratspeak Handheld. See [contributors](CONTRIBUTORS.md), [license](LICENSE)
and [third-party notices](THIRD_PARTY_NOTICES.md).
