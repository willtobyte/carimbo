.PHONY: help build clean configure debug release

BUILD_TYPE ?= Release
PROFILE ?= default

help:
	awk 'BEGIN {FS = ":.*?## "} /^[a-zA-Z_-]+:.*?## / {printf "\033[36m%-30s\033[0m %s\n", $$1, $$2}' $(MAKEFILE_LIST)

clean: ## Clean build files and remove marker file
	unalias rm 2>/dev/null || true
	rm -rf build 2>/dev/null

configure: ## Configure the project
	@if [ ! -f build/ready ]; then \
		$(MAKE) clean; \
		conan install . --output-folder=build --build="*" --profile=$(PROFILE) --settings compiler.cppstd=20 --settings build_type=$(BUILD_TYPE); \
		cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) -DHITBOX=ON -DLOCAL=ON; \
		mkdir -p build; \
		touch build/ready; \
	fi

build: ## Build the project
	cmake --build build --parallel 8 --config $(BUILD_TYPE) --verbose

debug: BUILD_TYPE=Debug
debug: configure build

release: BUILD_TYPE=Release
release: configure build
