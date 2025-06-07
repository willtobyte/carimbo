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
	@latest_tag=$$(git describe --tags --abbrev=0 2>/dev/null || echo "") ; \
	if [ -z "$$latest_tag" ]; then \
		echo "No tags found. Creating initial tag v0.0.1"; \
		git tag v0.0.1 && git push origin v0.0.1 ; \
	else \
		if git rev-list "$$latest_tag"..HEAD --quiet; then \
			base_version=$$(echo "$$latest_tag" | sed 's/^v//' | cut -d. -f1,2) ; \
			next_patch=$$(git tag | grep "^v$$base_version\." | sed "s/^v$$base_version\.//" | sort -n | tail -n1 | awk '{print $$1+1}') ; \
			new_tag="v$$base_version.$$next_patch" ; \
			echo "New commits found. Tagging as $$new_tag"; \
			git tag "$$new_tag" && git push origin "$$new_tag" ; \
		else \
			echo "No new commits since $$latest_tag. Skipping bump."; \
		fi ; \
	fi
