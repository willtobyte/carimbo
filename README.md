## Carimbo: A Modern, Cross-Platform 2D Game Engine

![macOS Build](https://github.com/${{ github.repository }}/actions/workflows/build.yml/badge.svg?branch=main&label=macOS&event=push&query=matrix.config.name%3AmacOS)
![Ubuntu Build](https://github.com/${{ github.repository }}/actions/workflows/build.yml/badge.svg?branch=main&label=Ubuntu&event=push&query=matrix.config.name%3AUbuntu)
![WebAssembly Build](https://github.com/${{ github.repository }}/actions/workflows/build.yml/badge.svg?branch=main&label=WebAssembly&event=push&query=matrix.config.name%3AWebAssembly)
![Windows Build](https://github.com/${{ github.repository }}/actions/workflows/build.yml/badge.svg?branch=main&label=Windows&event=push&query=matrix.config.name%3AWindows)

<p align="center">
  <img src="carimbo.avif" alt="Carimbo: A Modern, Cross-Platform 2D Game Engine">
</p>

### About

Carimbo is a simple yet complete 2D game engine written in modern C++ using SDL. It is scriptable in Lua and was created during [Rodrigo Delduca's](https://rodrigodelduca.org) spare time.

It is a spiritual successor to the [Wintermoon](https://github.com/wintermoon/wintermoon) framework, a project by the same author. It runs natively on Linux, Windows, macOS, and the web (via WebAssembly), and it also supports mobile platforms, including Android & iOS.

### Name

"Carimbo" comes from the 🇧🇷 word for "stamp," and that is exactly what a 2D game engine does—it continuously stamps sprites onto the screen.

### Games

Games and demos created with the Carimbo engine are hosted at [carimbo.run](https://carimbo.run). They can be tested or played online without installation, thanks to WebAssembly technology supported by all modern browsers.

### License

A simple, permissive license that offers complete commercial freedom—use, modify, and distribute your projects with ease—with minimal restrictions and a single attribution requirement. See [LICENSE](LICENSE).

### Building

See [BUILDING](BUILDING.md).
