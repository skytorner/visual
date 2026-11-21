# Developer shortcuts for the macOS AudioReactiveImagePOC POC.
# `make` checks the pure C++ DSP tests and builds all JUCE plugin formats.
# Keep DAW installation explicit: never overwrite an installed plugin on a build.

.DEFAULT_GOAL := all

CMAKE ?= cmake
CTEST ?= ctest
BUILD_TYPE ?= Release
BUILD_DIR ?= build
TEST_BUILD_DIR ?= build-tests

ARTIFACTS := $(BUILD_DIR)/AudioReactiveImagePOC_artefacts/$(BUILD_TYPE)
AU_BUNDLE := $(ARTIFACTS)/AU/AudioReactiveImagePOC.component
VST3_BUNDLE := $(ARTIFACTS)/VST3/AudioReactiveImagePOC.vst3
AU_INSTALL_DIR := $(HOME)/Library/Audio/Plug-Ins/Components
VST3_INSTALL_DIR := $(HOME)/Library/Audio/Plug-Ins/VST3

.PHONY: all check test plugin install install-au install-vst3 package status help

all: check

check: test plugin

# No JUCE fetch needed: fast, incremental C++17 analysis tests.
test:
	$(CMAKE) -S tests -B "$(TEST_BUILD_DIR)" -DCMAKE_BUILD_TYPE=$(BUILD_TYPE)
	$(CMAKE) --build "$(TEST_BUILD_DIR)" --config $(BUILD_TYPE)
	$(CTEST) --test-dir "$(TEST_BUILD_DIR)" --output-on-failure -C $(BUILD_TYPE)

# Reuses the CMake build directory; subsequent builds are incremental.
plugin:
	$(CMAKE) -S . -B "$(BUILD_DIR)" -DCMAKE_BUILD_TYPE=$(BUILD_TYPE)
	$(CMAKE) --build "$(BUILD_DIR)" --config $(BUILD_TYPE)

# Explicit installation: close Logic Pro / REAPER before using these targets.
install: install-au install-vst3

install-au: plugin
	@if [ "$$(uname -s)" != "Darwin" ]; then echo "install-au requires macOS"; exit 1; fi
	@test -d "$(AU_BUNDLE)" || { echo "AU bundle missing: $(AU_BUNDLE)"; exit 1; }
	@mkdir -p "$(AU_INSTALL_DIR)"
	@ditto "$(AU_BUNDLE)" "$(AU_INSTALL_DIR)/AudioReactiveImagePOC.component"
	@echo "Installed AU: $(AU_INSTALL_DIR)/AudioReactiveImagePOC.component"

install-vst3: plugin
	@if [ "$$(uname -s)" != "Darwin" ]; then echo "install-vst3 requires macOS"; exit 1; fi
	@test -d "$(VST3_BUNDLE)" || { echo "VST3 bundle missing: $(VST3_BUNDLE)"; exit 1; }
	@mkdir -p "$(VST3_INSTALL_DIR)"
	@ditto "$(VST3_BUNDLE)" "$(VST3_INSTALL_DIR)/AudioReactiveImagePOC.vst3"
	@echo "Installed VST3: $(VST3_INSTALL_DIR)/AudioReactiveImagePOC.vst3"

# Package for distribution, never as part of normal builds.
package: plugin
	@if [ "$$(uname -s)" != "Darwin" ]; then echo "package requires macOS"; exit 1; fi
	bash scripts/package-macos-release.sh

status:
	@git branch --show-current
	@git rev-parse --short HEAD

help:
	@echo "AV Sampler development commands (from repository root):"
	@echo "  make                Run C++ DSP tests and build JUCE plugin (AU/VST3/Standalone on macOS)"
	@echo "  make test           Run DSP tests only (fast)"
	@echo "  make plugin         Build plugin only (incremental)"
	@echo "  make install        Build and install AU + VST3 to user plugin folders (close DAWs first)"
	@echo "  make install-au     Build and install AU only"
	@echo "  make install-vst3   Build and install VST3 only"
	@echo "  make package        Build and create macOS ZIP in dist/"
	@echo "  make status         Show branch and commit for QA logs"
	@echo "  make help           Show these commands"
