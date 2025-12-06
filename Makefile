SHELL := /usr/bin/env bash
PROFILE := $(if $(profile),$(profile),default)
BUILDTYPE := $(if $(buildtype),$(buildtype),Debug)
SCENE := $(if $(SCENE),$(SCENE),prelude)
CARTRIDGE := $(if $(CARTRIDGE),$(CARTRIDGE),../reprobate)
NCPUS := 10

.SHELLFLAGS := -eu -o pipefail -c
.DEFAULT_GOAL := help

DEBUG_COMPILER_FLAGS := -Wpedantic -Werror -Wextra -Wno-unused-parameter -Wshadow -Wconversion -Wsign-conversion -Wimplicit-fallthrough -Wdouble-promotion -Wformat=2 -Wnull-dereference -Wnon-virtual-dtor -fsanitize=address,undefined -fsanitize-address-use-after-scope -fno-omit-frame-pointer
DEBUG_LINKER_FLAGS := -g3 -fno-optimize-sibling-calls -fsanitize=address,undefined -fsanitize-address-use-after-scope -fno-omit-frame-pointer

.PHONY: clean
clean: ## Cleans build artifacts
	rm -rf build
	rm -rf ~/.conan2/p

.PHONY: conan
conan: ## Installs dependencies
	conan install . --output-folder=build --build=missing --profile=$(PROFILE) --settings build_type=$(BUILDTYPE)

.PHONY: build
build: ## Builds the project
	cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=build/conan_toolchain.cmake \
	-DDEVELOPMENT=TRUE \
	-DCMAKE_BUILD_TYPE=$(BUILDTYPE) \
	-DCMAKE_C_FLAGS_DEBUG="$(DEBUG_COMPILER_FLAGS)" \
	-DCMAKE_CXX_FLAGS_DEBUG="$(DEBUG_COMPILER_FLAGS)" \
	-DCMAKE_EXE_LINKER_FLAGS_DEBUG="$(DEBUG_LINKER_FLAGS)" \
	$(EXTRA_FLAGS)

	cmake --build build --parallel $(NCPUS) --config $(BUILDTYPE) --verbose

.PHONY: run
run: build ## Builds and runs the project
	NOVSYNC=1 SCENE=$(SCENE) CARTRIDGE=$(CARTRIDGE) lldb -o run -- ./build/carimbo

.PHONY: help
help:
	@awk 'BEGIN {FS = ":.*?## "} /^[a-zA-Z_-]+:.*?## / {printf "\033[36m%-30s\033[0m %s\n", $$1, $$2}' $(MAKEFILE_LIST)
