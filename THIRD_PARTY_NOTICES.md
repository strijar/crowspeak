# Third-party notices

The firmware and build tools use the [GNU Affero General Public License v3.0 or later](LICENSE).
Third-party code and assets retain their own licenses.

The [notice bundle](licenses/THIRD-PARTY.txt) and [inventory](licenses/manifest.json)
include Rust dependencies, Arduino/ESP-IDF components, graphics, fonts and compiler
runtimes. Conservative inherited SDK notice entries record the source of their
license texts, not the identity of Elecrow's precompiled libraries. Exact SDK
inputs are recorded in [the SDK manifest](vendor/crowpanel-sdk/manifest.json).
The bundle also preserves the license and copyright headers shipped in that SDK.

CrowPanel fonts derive from Montserrat (OFL); LVGL symbol fonts use Font Awesome.
Original notices remain in the bundled sources and archive. The Arduino 3.0.2
Wi-Fi adaptations are applied to build-local source copies by
`tools/patch_crowpanel_wifi.py`.

Redistributions must retain applicable notices and corresponding source required
by each license. The protocol source revisions are recorded in
[PROVENANCE.txt](protocol/prebuilt/xtensa-esp32s3/PROVENANCE.txt).

Verify the committed inventory with `python tools/collect_licenses.py --check`.
Dependency upgrades require reviewing the notice texts and inventory together.
