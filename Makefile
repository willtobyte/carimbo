SHELL := /usr/bin/env bash
PROFILE := $(if $(profile),$(profile),default)
BUILDTYPE := $(if $(filter debug,$(buildtype)),Debug,Release)

.PHONY: install
install: ## Install dependencies
	conan install . --output-folder=build --build=missing --profile=$(PROFILE) --settings build_type=$(BUILDTYPE)

.PHONY: help
help:
	@awk 'BEGIN {FS = ":.*?## "} /^[a-zA-Z_-]+:.*?## / {printf "\033[36m%-30s\033[0m %s\n", $$1, $$2}' $(MAKEFILE_LIST)
