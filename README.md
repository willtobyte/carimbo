## Carimbo: A Modern, Cross-Platform 2D Game Engine

<p align="center">
  <img src="carimbo.avif" alt="Carimbo: A Modern, Cross-Platform 2D Game Engine">
</p>

### About

Carimbo is a simple yet complete 2D game engine written in modern C++ using SDL. It is scriptable in Lua and was created during the spare time of [Rodrigo Delduca](https://rodrigodelduca.com.br/).

It is a spiritual successor to the [Wintermoon](https://github.com/wintermoon/wintermoon) framework, a project by the same author. It runs natively on Linux, Windows, macOS, and on the web (via WebAssembly), and it also supports mobile platforms, including Android & iOS.

### Motivation

Roughly 15 years ago, during an extended summer holiday, in a shared room of a student residence, I found myself endeavoring to port my 2D game engine built on top of SDL to the Google Native Client (NaCl). NaCl served as a sandboxing mechanism for Chrome, enabling the execution of native code within the browser, specifically within the Chrome browser. It is safe to assert that NaCl can be considered the progenitor of WebAssembly.

A considerable amount of time has elapsed, and many changes have transpired. I transitioned from game development to web development, yet low-level programming has always coursed through my veins. Consequently, I resolved to revive the dream of crafting my own game engine running on the web.

Today, with the advent of WebAssembly, achieving this goal is significantly more feasible and portable.

### Name

_"Carimbo"_ comes from the 🇧🇷 word for "stamp," and that is exactly what a 2D game engine does—it constantly stamps sprites onto the screen.

### Games

Games and demos created with the Carimbo engine are hosted on https://carimbo.run. They can be tested or played online without any installation, thanks to WebAssembly technology native to all modern browsers.

### License

A simple, permissive license that offers complete commercial freedom—use, modify, and distribute your projects with ease. With minimal restrictions and a single attribution requirement. See [LICENSE](LICENSE).

### Building

See [BUILDING](BUILDING.md).
