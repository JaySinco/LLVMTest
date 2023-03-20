import sys, os
from myconanfile import MyConanFile
from conans import ConanFile, tools
from conan.tools.cmake import CMakeToolchain, CMake
from conan.tools.files import collect_libs, copy, rmdir
import os


class ImguiConan(MyConanFile):
    name = "imgui"
    version = "1.87"
    homepage = "https://github.com/ocornut/imgui"
    description = "Bloat-free Immediate Mode Graphical User interface for C++ with minimal dependencies"
    license = "MIT"

    settings = "os", "arch", "compiler", "build_type"
    options = {
        "shared": [True, False],
        "fPIC": [True, False],
    }
    default_options = {
        "shared": False,
        "fPIC": True,
    }

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def configure(self):
        if self.options.shared:
            del self.options.fPIC

    def source(self):
        srcFile = self._src_abspath(f"{self.name}-{self.version}.tar.gz")
        tools.unzip(srcFile, destination=self.source_folder, strip_root=True)
        copy(self, "CMakeLists.txt", dst=self.source_folder, src=self._dirname(__file__))

    def generate(self):
        tc = CMakeToolchain(self)
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        copy(self, "LICENSE.txt", dst=os.path.join(
            self.package_folder, "licenses"), src=self.source_folder)
        copy(self, pattern="imgui_impl_*",
             dst=os.path.join(self.package_folder, "res", "bindings"),
             src=os.path.join(self.source_folder, "backends"))
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.set_property("cmake_file_name", "imgui")
        self.cpp_info.set_property("cmake_target_name", "imgui::imgui")
        self.cpp_info.set_property("pkg_config_name", "imgui")
        self.cpp_info.libs = collect_libs(self, folder="lib")
        self.cpp_info.srcdirs = [os.path.join("res", "bindings")]
        if self.settings.os in ["Linux", "FreeBSD"]:
            self.cpp_info.system_libs.extend(["m"])
