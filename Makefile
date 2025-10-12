SHELL := /usr/bin/env bash
PROFILE := $(if $(profile),$(profile),default)
BUILDTYPE := $(if $(buildtype),$(buildtype),Release)
NCPUS := 8

.SHELLFLAGS := -eu -o pipefail -c
.DEFAULT_GOAL := help

ifeq ($(PROFILE),webassembly)
EXTRA_FLAGS := -DSANDBOX=OFF
else
EXTRA_FLAGS := -DSANDBOX=ON
endif

COMPILER_FLAGS := -Wpedantic -Werror -fsanitize=address,undefined -fsanitize-address-use-after-scope -fno-omit-frame-pointer
LINKER_FLAGS := -fsanitize=address,undefined

.PHONY: clean
clean: ## Cleans build artifacts
	@rm -rf build
	@rm -rf ~/.conan2/p

.PHONY: conan
conan: ## Installs dependencies
	conan install . --output-folder=build --build=missing --profile=$(PROFILE) --settings build_type=$(BUILDTYPE)

.PHONY: build
build: ## Builds the project
	cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=build/conan_toolchain.cmake -DCMAKE_BUILD_TYPE=$(BUILDTYPE) \
	-DCMAKE_C_FLAGS_DEBUG="$(COMPILER_FLAGS)" \
	-DCMAKE_CXX_FLAGS_DEBUG="$(COMPILER_FLAGS)" \
	-DCMAKE_EXE_LINKER_FLAGS_DEBUG="$(LINKER_FLAGS)" \
	$(EXTRA_FLAGS)

	cmake --build build --parallel $(NCPUS) --config $(BUILDTYPE) --verbose

.PHONY: help
help:
	@awk 'BEGIN {FS = ":.*?## "} /^[a-zA-Z_-]+:.*?## / {printf "\033[36m%-30s\033[0m %s\n", $$1, $$2}' $(MAKEFILE_LIST)
