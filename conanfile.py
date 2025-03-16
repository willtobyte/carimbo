from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMakeDeps


class Carimbo(ConanFile):
    settings = "os", "arch", "compiler", "build_type"

    def requirements(self):
        self.requires("fmt/11.1.3")
        self.requires("libspng/0.7.4")
        self.requires("nlohmann_json/3.11.3")
        self.requires("ogg/1.3.5")
        self.requires("openal-soft/1.23.1")
        self.requires("physfs/3.2.0")
        self.requires("sdl/2.30.9")
        self.requires("sol2/3.3.1")
        self.requires("vorbis/1.3.7")

        if self.settings.os != "Emscripten":
            self.requires("boost/1.86.0")
            self.requires("libressl/3.9.1")

    def generate(self):
        tc = CMakeToolchain(self)
        tc.generate()
        deps = CMakeDeps(self)
        deps.generate()
