SHELL := /usr/bin/env bash
PROFILE := $(if $(profile),$(profile),default)
BUILDTYPE := $(if $(filter debug,$(buildtype)),Debug,Release)
NCPUS := $(shell \
  (command -v nproc >/dev/null 2>&1 && nproc) || \
  (command -v sysctl >/dev/null 2>&1 && sysctl -n hw.ncpu 2>/dev/null) || \
  (command -v getconf >/dev/null 2>&1 && getconf _NPROCESSORS_ONLN 2>/dev/null) || \
  echo 8 \
)

.SHELLFLAGS := -eu -o pipefail -c
.DEFAULT_GOAL := help
.DELETE_ON_ERROR:
.SUFFIXES:

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
	 cmake --build build --parallel $(NCPUS) --config $(BUILDTYPE) --verbose

.PHONY: help
help:
	 @awk 'BEGIN {FS = ":.*?## "} /^[a-zA-Z_-]+:.*?## / {printf "\033[36m%-30s\033[0m %s\n", $$1, $$2}' $(MAKEFILE_LIST)

.PHONY: bump
bump: ## Bump git
	@set -e; \
	latest_tag=$$(git tag --list 'v*' | sort -V | tail -n1); \
	if [ -z "$$latest_tag" ]; then \
		echo "No tags found. Creating initial tag v0.0.1"; \
		git tag v0.0.1; \
		git push origin v0.0.1; \
	elif [ "$$(git rev-parse HEAD)" = "$$(git rev-list -n1 $$latest_tag)" ]; then \
		echo "HEAD is already tagged with $$latest_tag. No new tag needed."; \
	else \
		base_version=$$(echo "$$latest_tag" | sed 's/^v//' | cut -d. -f1,2); \
		last_patch=$$(git tag | grep -E "^v$$base_version\\.[0-9]+$$" | sed "s/^v$$base_version\.//" | sort -n | tail -n1); \
		if [ -z "$$last_patch" ]; then next_patch=0; else next_patch=$$((last_patch + 1)); fi; \
		new_tag="v$$base_version.$$next_patch"; \
		while git rev-parse "$$new_tag" >/dev/null 2>&1; do \
			next_patch=$$((next_patch + 1)); \
			new_tag="v$$base_version.$$next_patch"; \
		done; \
		echo "Tagging new release: $$new_tag"; \
		git tag "$$new_tag"; \
		git push origin "$$new_tag"; \
	fi
