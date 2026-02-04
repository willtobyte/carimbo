from conan import ConanFile
from conan.tools.cmake import CMakeDeps, CMakeToolchain
from pathlib import Path


class Carimbo(ConanFile):
    settings = "os", "arch", "compiler", "build_type"

    @property
    def _os_name(self):
        return str(self.settings.os).lower()

    @property
    def _is_webassembly(self):
        return self._os_name == "emscripten"

    @property
    def _is_jit_capable(self):
        return self._os_name in {"linux", "macos", "windows"}

    @property
    def _is_ios(self):
        return self._os_name == "ios"

    @property
    def _has_steam(self):
        return self._os_name in {"macos", "windows"}

    @property
    def _has_sentry(self):
        return self._os_name in {"macos", "windows"}

    def requirements(self):
        for package in [
            "boost/1.90.0",
            "box2d/3.1.1",
            "entt/3.16.0",
            "yyjson/0.12.0",
            "openal-soft/1.23.1",
            "physfs/3.2.0",
            "libspng/0.7.4",
            "sdl/3.2.20",
            "sol2/3.5.0",
            "stb/cci.20240531",
        ]:
            self.requires(package)

        for package, condition in [
            ("luajit/2.1.0-beta3", self._is_jit_capable),
            ("sentry-native/0.12.2", self._has_sentry),
            ("openssl/3.6.0", not self._is_webassembly),
        ]:
            if condition:
                self.requires(package)

    def configure(self):
        self.options["boost"].header_only = True

        self.options["sol2"].with_lua = "luajit" if self._is_jit_capable else "lua"

        for opt in [
            "sevenzip",
            "grp",
            "wad",
            "hog",
            "mvl",
            "qpak",
            "slb",
            "iso9660",
            "vdf",
        ]:
            setattr(self.options["physfs"], opt, False)

        if self._is_ios:
            self.options["sdl"].opengl = False

        if self._has_sentry:
            self.options["sentry-native"].backend = "crashpad"
            self.options["sentry-native"].with_crashpad = "sentry"
            self.options["sentry-native"].shared = False

    def generate(self):
        license_output = Path(self.build_folder) / "LICENSES"
        with license_output.open("w", encoding="utf-8") as out:
            for dep in self.dependencies.values():
                if dep.is_build_context or not dep.package_folder:
                    continue

                pid = f"{dep.ref.name}/{dep.ref.version}"
                for file in Path(dep.package_folder).rglob("*"):
                    if not file.is_file():
                        continue

                    name = file.name.lower()
                    if name.startswith(("license", "copying", "copyright")):
                        text = file.read_text(encoding="utf-8", errors="ignore").strip()
                        out.write(f"{pid}\n{text}\n\n")

        toolchain = CMakeToolchain(self)

        for flag, condition in [
            ("HAS_LUAJIT", self._is_jit_capable),
            ("HAS_STEAM", self._has_steam),
            ("HAS_SENTRY", self._has_sentry),
        ]:
            if condition:
                toolchain.preprocessor_definitions[flag] = "ON"
                toolchain.cache_variables[flag] = "ON"

        toolchain.generate()
        CMakeDeps(self).generate()
