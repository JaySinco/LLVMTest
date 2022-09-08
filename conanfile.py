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
        "gflags:nothreads": False,
    }

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def configure(self):
        if self.options.shared:
            del self.options.fPIC

    def requirements(self):
        self.requires("spdlog/1.10.0@jaysinco/stable")
        self.requires("boost/1.79.0@jaysinco/stable")
        self.requires("glfw/3.3.7@jaysinco/stable")
        self.requires("implot/0.13@jaysinco/stable")
        self.requires("expected-lite/0.5.0@jaysinco/stable")
        self.requires("catch2/2.13.9@jaysinco/stable")
        self.requires("mujoco/2.1.5@jaysinco/stable")
        self.requires("torch/1.8.2@jaysinco/stable")
        self.requires("qt/5.15.3@jaysinco/stable")

    def layout(self):
        build_folder = "out"
        build_type = str(self.settings.build_type)
        self.folders.source = ""
        self.folders.build = os.path.join(build_folder, build_type)
        self.folders.generators = os.path.join(
            self.folders.build, "generators")

    def generate(self):
        tc = CMakeToolchain(self)
        tc.variables["CMAKE_RUNTIME_OUTPUT_DIRECTORY"] = os.path.join(
            self.source_folder, "bin").replace("\\", "/")
        tc.variables["CMAKE_PREFIX_PATH"] = self._cmake_path()
        self._setup_pkg_root(tc)
        tc.generate()
        cmake_deps = CMakeDeps(self)
        cmake_deps.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def _setup_pkg_root(self, tc):
        pkg_list = ["imgui", "mujoco", "torch"]
        for pkg in pkg_list:
            macro = "{}_ROOT".format(pkg.upper())
            tc.variables[macro] = self._normalize(
                self.deps_cpp_info[pkg].cpp_info.rootpath)

    def _cmake_path(self):
        prefix_path = []
        cmake_dir = {
            "boost" : "lib/cmake",
            "qt" : "lib/cmake",
        }
        for pkg in cmake_dir:
            prefix_path.append(self._normalize(
                os.path.join(self.deps_cpp_info[pkg].cpp_info.rootpath, cmake_dir[pkg])))

        return "%s;${CMAKE_PREFIX_PATH}" % (";".join(prefix_path))

    def _normalize(self, path):
        if self.settings.os == "Windows":
            return path.replace("\\", "/")
        else:
            return path
