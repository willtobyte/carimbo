from conan import ConanFile
from conan.tools.cmake import CMakeDeps
from conan.tools.cmake import CMakeToolchain


class Carimbo(ConanFile):
    settings = "os", "arch", "compiler", "build_type"

    def _not_webassembly(self):
        return str(self.settings.os).lower() not in {"emscripten"}

    def configure(self):
        if self._not_webassembly():
            self.options["sol2"].with_lua = "luajit"

    def requirements(self):
        self.requires("fmt/11.1.4")
        self.requires("libspng/0.7.4")
        self.requires("nlohmann_json/3.11.3")
        self.requires("ogg/1.3.5")
        self.requires("openal-soft/1.23.1")
        self.requires("physfs/3.2.0")
        self.requires("sdl/3.2.6")
        self.requires("sol2/3.5.0")
        self.requires("vorbis/1.3.7")

        self.requires("libalsa/1.2.12", override=True)

        if self._not_webassembly():
            self.requires("boost/1.87.0")
            self.requires("luajit/2.1.0-beta3")
            self.requires("openssl/3.4.1")

    def generate(self):
        tc = CMakeToolchain(self)
        tc.generate()
        deps = CMakeDeps(self)
        deps.generate()
