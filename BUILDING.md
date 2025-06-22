## Project Setup

This guide provides a clear and minimal step-by-step process for setting up and building the project.

### Install Tools

Install Conan using [mise](https://mise.jdx.dev):

```shell
mise use --global conan
```

Detect your system profile:

```shell
conan profile detect --force
```

Edit your default Conan profile:

```shell
vim ~/.conan2/profiles/default
```

Append the following configuration at the end of the file:

```
[replace_tool_requires]
meson/*: meson/[*]
pkgconf/*: pkgconf/[*]

[tool_requires]
!cmake/*: cmake/[>=3 <4]
```

### Build Instructions

First-time build (includes Conan dependency resolution):

```shell
make conan build
```

Subsequent builds (rebuild without re-resolving dependencies):

```shell
make build
```
