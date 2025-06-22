## Project Setup

This guide provides a straightforward walkthrough for setting up and building the project. Follow the instructions below to get started.

### Install Mise & Conan

Set up the Python virtual environment:

```shell
mise use --global conan
```

Create your profile:

```shell
conan profile detect --force
```

```shell
vim ~/.conan2/profiles/default
```

Add this at the end:

```
[replace_tool_requires]
meson/*: meson/[*]
pkgconf/*: pkgconf/[*]

[tool_requires]
!cmake/*: cmake/[>=3 <4]
```

### Configure & Build

On the first build:

```shell
make conan build
```

On subsequent builds:

```shell
make build
```
