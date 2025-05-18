from conan import ConanFile
from conan.tools.cmake import CMakeDeps
from conan.tools.cmake import CMakeToolchain
from pathlib import Path


class Carimbo(ConanFile):
    settings = "os", "arch", "compiler", "build_type"

    def _not_webassembly(self):
        return str(self.settings.os).lower() not in {"emscripten"}

    def _jit_capable(self):
        return str(self.settings.os).lower() in {"windows", "macos", "linux"}

    def requirements(self):
        self.requires("fmt/11.1.4")
        self.requires("libspng/0.7.4")
        self.requires("nlohmann_json/3.12.0")
        self.requires("ogg/1.3.5")
        self.requires("openal-soft/1.23.1")
        self.requires("physfs/3.2.0")
        self.requires("sdl/3.2.6")
        self.requires("sol2/3.5.0")
        self.requires("vorbis/1.3.7")

        if self._not_webassembly():
            self.requires("boost/1.87.0")
            self.requires("openssl/3.4.1")

            if self._jit_capable():
              self.requires("luajit/2.1.0-beta3")

    def configure(self):
        self.options["boost"].header_only = True

        self.options["physfs"].sevenzip = True
        self.options["physfs"].zip = False
        self.options["physfs"].grp = False
        self.options["physfs"].wad = False
        self.options["physfs"].hog = False
        self.options["physfs"].mvl = False
        self.options["physfs"].qpak = False
        self.options["physfs"].slb = False
        self.options["physfs"].iso9660 = False
        self.options["physfs"].vdf = False

        if self._not_webassembly() and self._jit_capable():
            self.options["sol2"].with_lua = "luajit"

    def generate(self):
        path = Path(self.build_folder) / "LICENSES"
        with path.open("w", encoding="utf-8") as out:
            for dependecy in self.dependencies.values():
                if dependecy.is_build_context or dependecy.package_folder is None:
                    continue
                package_id = f"{dependecy.ref.name}/{dependecy.ref.version}"
                for path in Path(dependecy.package_folder).rglob("*"):
                    if not path.is_file():
                        continue
                    name = path.name.lower()
                    if name.startswith(("license", "copying", "copyright")):
                        text = path.read_text(encoding="utf-8", errors="ignore").strip()
                        out.write(f"{package_id}\n{text}\n\n")

        tc = CMakeToolchain(self)

        if self._not_webassembly():
            tc.preprocessor_definitions["HAVE_BOOST"] = "1"

        if self._not_webassembly() and self._jit_capable():
            tc.preprocessor_definitions["HAVE_LUAJIT"] = "1"

        tc.generate()
        deps = CMakeDeps(self)
        deps.generate()
