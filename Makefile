SHELL := /usr/bin/env bash
PROFILE := $(if $(profile),$(profile),default)
BUILDTYPE := $(if $(filter debug,$(buildtype)),Debug,Release)

ifeq ($(PROFILE),webassembly)
	EXTRA_FLAGS := -DHITBOX=ON
else
	EXTRA_FLAGS := -DHITBOX=ON -DSANDBOX=ON
endif

.PHONY: clean
clean: ## Clean
	 rm -rf build
	 rm -rf ~/.conan2/p

.PHONY: conan
conan: ## Install dependencies
	 conan install . --output-folder=build --build=missing --profile=$(PROFILE) --settings build_type=$(BUILDTYPE)

.PHONY: build
build: ## Build
	 cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -DCMAKE_BUILD_TYPE=$(BUILDTYPE) $(EXTRA_FLAGS)
	 cmake --build build --parallel 8 --config $(BUILDTYPE) --verbose

.PHONY: help
help:
	 @awk 'BEGIN {FS = ":.*?## "} /^[a-zA-Z_-]+:.*?## / {printf "\033[36m%-30s\033[0m %s\n", $$1, $$2}' $(MAKEFILE_LIST)
