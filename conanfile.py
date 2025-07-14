from conan import ConanFile
from conan.tools.cmake import CMakeDeps, CMakeToolchain
from pathlib import Path


class Carimbo(ConanFile):
    settings = "os", "arch", "compiler", "build_type"

    def _os_name(self):
        return str(self.settings.os).lower()

    def _is_webassembly(self):
        return self._os_name() == "emscripten"

    def _is_jit_capable(self):
        return self._os_name() in {"linux", "macos", "windows"}

    def _is_ios(self):
        return self._os_name() == "ios"

    def _have_steam(self):
        return self._os_name() in {"macos", "windows"}

    def requirements(self):
        self.requires("libspng/0.7.4")
        self.requires("nlohmann_json/3.12.0")
        self.requires("ogg/1.3.5")
        self.requires("openal-soft/1.23.1")
        self.requires("physfs/3.2.0")
        self.requires("sdl/3.2.14")
        self.requires("sol2/3.5.0")
        self.requires("vorbis/1.3.7")

        if self._is_webassembly():
            return

        self.requires("boost/1.87.0")
        self.requires("openssl/3.4.1")

        if self._is_jit_capable():
            self.requires("luajit/2.1.0-beta3")

    def configure(self):
        self.options["boost"].header_only = True

        physfs = self.options["physfs"]
        physfs.sevenzip = False
        physfs.grp = False
        physfs.wad = False
        physfs.hog = False
        physfs.mvl = False
        physfs.qpak = False
        physfs.slb = False
        physfs.iso9660 = False
        physfs.vdf = False
        physfs.zip = True

        if not self._is_webassembly() and self._is_jit_capable():
            self.options["sol2"].with_lua = "luajit"

        if self._is_ios():
            self.options["sdl"].opengl = False

    def generate(self):
        license_output = Path(self.build_folder) / "LICENSES"
        with license_output.open("w", encoding="utf-8") as out:
            for dep in self.dependencies.values():
                if dep.is_build_context or not dep.package_folder:
                    continue

                package_id = f"{dep.ref.name}/{dep.ref.version}"
                for file in Path(dep.package_folder).rglob("*"):
                    if not file.is_file():
                        continue

                    name = file.name.lower()
                    if name.startswith(("license", "copying", "copyright")):
                        text = file.read_text(encoding="utf-8", errors="ignore").strip()
                        out.write(f"{package_id}\n{text}\n\n")

        toolchain = CMakeToolchain(self)

        if not self._is_webassembly():
            toolchain.preprocessor_definitions["HAVE_BOOST"] = None

        if self._is_jit_capable():
            toolchain.preprocessor_definitions["HAVE_LUAJIT"] = None

        if self._have_steam():
            toolchain.preprocessor_definitions["HAVE_STEAM"] = None

        toolchain.generate()
        CMakeDeps(self).generate()
