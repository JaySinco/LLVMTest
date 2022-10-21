from conans import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake, CMakeDeps
import os
import sys


class PrototypingConan(ConanFile):
    name = "prototyping"
    version = "0.1.0"
    url = "https://github.com/JaySinco/conan"
    homepage = "https://github.com/JaySinco/Prototyping"
    description = "C++ prototype repo"
    license = "MIT"

    settings = "os", "arch", "compiler", "build_type"
    options = {
        "shared": [True, False],
        "fPIC": [True, False],
    }
    default_options = {
        "shared": False,
        "fPIC": True,
        "spdlog:no_exceptions": False,
    }

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def configure(self):
        if self.options.shared:
            del self.options.fPIC

    def requirements(self):
        self.requires(self._ref_pkg("spdlog/1.10.0"))
        self.requires(self._ref_pkg("argparse/2.9"))
        self.requires(self._ref_pkg("boost/1.79.0"))
        self.requires(self._ref_pkg("glfw/3.3.7"))
        self.requires(self._ref_pkg("implot/0.13"))
        self.requires(self._ref_pkg("expected-lite/0.5.0"))
        self.requires(self._ref_pkg("catch2/2.13.9"))
        self.requires(self._ref_pkg("mujoco/2.2.2"))
        self.requires(self._ref_pkg("torch/1.8.2"))
        self.requires(self._ref_pkg("qt/5.15.6"))
        self.requires(self._ref_pkg("range-v3/0.12.0"))
        self.requires(self._ref_pkg("libiconv/1.17"))
        self.requires(self._ref_pkg("raylib/4.2.0"))

    def layout(self):
        build_folder = "out"
        build_type = str(self.settings.build_type)
        self.folders.source = ""
        self.folders.build = os.path.join(build_folder, build_type)
        self.folders.generators = os.path.join(
            self.folders.build, "generators")

    def generate(self):
        tc = CMakeToolchain(self)
        tc.variables["CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG"] = self._normalize_path(
            os.path.join(self.source_folder, "bin"))
        tc.variables["CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE"] = self._normalize_path(
            os.path.join(self.source_folder, "bin", "Release"))
        tc.variables["CMAKE_PREFIX_PATH"] = self._cmake_path()
        tc.variables["SOURCE_REPO"] = self._normalize_path(
            os.getenv('JAYSINCO_SOURCE_REPO'))
        self._setup_pkg_root(tc)
        tc.generate()
        cmake_deps = CMakeDeps(self)
        cmake_deps.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def _setup_pkg_root(self, tc):
        for pkg in self.deps_cpp_info._dependencies:
            root = "{}_ROOT".format(pkg)
            tc.variables[root] = self._normalize_path(
                self.deps_cpp_info[pkg].cpp_info.rootpath)

    def _cmake_path(self):
        prefix_path = []
        cmake_dir = {
            "qt" : "lib/cmake",
        }
        for pkg in cmake_dir:
            prefix_path.append(self._normalize_path(
                os.path.join(self.deps_cpp_info[pkg].cpp_info.rootpath, cmake_dir[pkg])))

        return "%s;${CMAKE_PREFIX_PATH}" % (";".join(prefix_path))

    def _ref_pkg(self, pkgname: str):
        return f"{pkgname}@jaysinco/stable"

    def _normalize_path(self, path):
        if self.settings.os == "Windows":
            return path.replace("\\", "/")
        else:
            return path
