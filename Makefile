SHELL := /usr/bin/env bash
PROFILE := $(if $(profile),$(profile),default)
BUILDTYPE := $(if $(buildtype),$(buildtype),Release)
NCPUS := 8

.SHELLFLAGS := -eu -o pipefail -c
.DEFAULT_GOAL := help

ifeq ($(PROFILE),webassembly)
EXTRA_FLAGS := -DHITBOX=ON -DSANDBOX=OFF
else
EXTRA_FLAGS := -DHITBOX=ON -DSANDBOX=ON
endif

.PHONY: clean
clean: ## Limpa os arquivos de build
	@rm -rf build
	@rm -rf ~/.conan2/p

.PHONY: conan
conan: ## Instala as dependências
	@conan install . --output-folder=build --build=missing --profile=$(PROFILE) --settings build_type=$(BUILDTYPE)

.PHONY: build
build: ## Compila o projeto
	@cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=build/conan_toolchain.cmake -DCMAKE_BUILD_TYPE=$(BUILDTYPE) $(EXTRA_FLAGS)
	@cmake --build build --parallel $(NCPUS) --config $(BUILDTYPE) --verbose

.PHONY: help
help:
	@awk 'BEGIN {FS = ":.*?## "} /^[a-zA-Z_-]+:.*?## / {printf "\033[36m%-30s\033[0m %s\n", $$1, $$2}' $(MAKEFILE_LIST)
