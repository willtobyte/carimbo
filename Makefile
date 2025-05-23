SHELL := /usr/bin/env bash
SHELLFLAGS = -euo pipefail -c
PROFILE := $(if $(profile),$(profile),default)
BUILDTYPE := $(if $(filter debug,$(buildtype)),Debug,Release)
NCPUS := $(shell sysctl -n hw.ncpu)

ifeq ($(PROFILE),webassembly)
	EXTRA_FLAGS := -DHITBOX=ON
else
	EXTRA_FLAGS := -DHITBOX=ON -DSANDBOX=ON
endif

.DEFAULT_GOAL := help

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
	 cmake --build build --parallel $(NCPUS) --config $(BUILDTYPE) --verbose

.PHONY: help
help:
	 @awk 'BEGIN {FS = ":.*?## "} /^[a-zA-Z_-]+:.*?## / {printf "\033[36m%-30s\033[0m %s\n", $$1, $$2}' $(MAKEFILE_LIST)

.PHONY: bump
bump: ## Bump git tag
	@tag=$$(git tag | sort -V | tail -n1 | sed 's/^v//' | awk -F. '{print "v"$$1"."$$2"."($$3+1)}') && git tag "$$tag" && git push origin --tags
