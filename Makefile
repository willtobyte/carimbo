.PHONY: help build clean configure

SHELL := bash -eoux pipefail

ifeq ($(shell command -v docker-compose;),)
	COMPOSE := docker compose
else
	COMPOSE := docker-compose
endif

help:
	awk 'BEGIN {FS = ":.*?## "} /^[a-zA-Z_-]+:.*?## / {printf "\033[36m%-30s\033[0m %s\n", $$1, $$2}' $(MAKEFILE_LIST)

clean: ## Clean
	unalias rm 2>/dev/null || true
	rm -rf build 2>/dev/null

build: ## Build
	cmake --build build --parallel 8

configure: clean ## Configure
	conan remote update conancenter --url https://center2.conan.io
	conan install . --output-folder=build --build=missing --profile=webassembly --settings compiler.cppstd=20 --settings build_type=Release
	cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -DSDL2_DIR=generators -DCMAKE_BUILD_TYPE=Release -DLOCAL=ON -DSANDBOX=OFF -DENABLE_PROFILING=ON
