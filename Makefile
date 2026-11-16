# Use the project's build environment without requiring shell activation.
PYTHON ?= $(if $(wildcard .venv/bin/python),.venv/bin/python,python3)
PLATFORMIO_CORE_DIR ?= $(CURDIR)/.platformio-core
export PLATFORMIO_CORE_DIR
DEVICE ?= crowpanel
SUPPORTED_DEVICES := $(shell $(PYTHON) tools/release_catalog.py devices)
STANDALONE_ENV ?= $(DEVICE)
BUILD_DIR ?= build
DIST_DIR ?= dist
PORT ?= /dev/ttyUSB0
UPLOAD_BAUD ?= 115200

ifeq ($(filter $(DEVICE),$(SUPPORTED_DEVICES)),)
$(error Unsupported DEVICE '$(DEVICE)'; choose one of: $(SUPPORTED_DEVICES))
endif

.PHONY: all build setup doctor doctor-source source-check build-standalone ui-check check check-all bundle standalone-image package package-all protocol-check software-check release flash clean
all: bundle
build: package
setup:
	$(PYTHON) -m platformio pkg install -e $(DEVICE)
doctor:
	$(PYTHON) tools/doctor.py --device $(DEVICE)
doctor-source:
	$(PYTHON) tools/doctor.py --device $(DEVICE) --source
	$(PYTHON) tools/check_prebuilt.py --allow-dirty
source-check:
	$(PYTHON) tools/release_catalog.py check
	$(PYTHON) tools/check_source_release.py
	$(PYTHON) tools/check_runtime_boundary.py
	$(PYTHON) tools/collect_licenses.py --check
	$(PYTHON) tools/check_crowpanel_sdk.py
build-standalone:
	$(PYTHON) -m platformio run -e $(STANDALONE_ENV)
ui-check:
	$(PYTHON) tools/check_ui_styles.py
check: build-standalone
	$(PYTHON) tools/check_image_fit.py --device $(DEVICE) --standalone .pio/build/$(STANDALONE_ENV)/firmware.bin
check-all: check
standalone-image: check
	mkdir -p $(BUILD_DIR)
	cp .pio/build/$(STANDALONE_ENV)/crowpanel-standalone-factory.bin $(BUILD_DIR)/crowpanel-standalone.bin
bundle: standalone-image
package: standalone-image
	mkdir -p $(DIST_DIR)
	$(PYTHON) tools/package_merged_zip.py --image $(BUILD_DIR)/crowpanel-standalone.bin --name crowpanel-standalone --device $(DEVICE) --package standalone --flash-size 16MB --output $(DIST_DIR)/crowpanel-standalone.zip
	cp $(BUILD_DIR)/crowpanel-standalone.bin $(DIST_DIR)/crowpanel-standalone.bin
package-all: package
protocol-check: source-check
	cd protocol && cargo fmt --all -- --check
	cd protocol && cargo test --workspace --locked
	cd protocol && cargo clippy --workspace --no-default-features --features 'std profile-small' --all-targets --locked -- -D warnings
	$(PYTHON) tools/check_prebuilt.py --allow-dirty
software-check: protocol-check check-all
release: package
flash: bundle
	$(PYTHON) -m esptool --chip esp32s3 --port $(PORT) --baud $(UPLOAD_BAUD) --before default_reset --after hard_reset write-flash 0x0 $(BUILD_DIR)/crowpanel-standalone.bin
monitor: bundle
	$(PYTHON) -m platformio device monitor --port $(PORT) --baud $(UPLOAD_BAUD)
clean:
	rm -rf $(BUILD_DIR) $(DIST_DIR)
	$(PYTHON) -m platformio run -e $(DEVICE) -t clean
