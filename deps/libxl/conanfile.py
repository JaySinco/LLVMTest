import sys, os
from myconanfile import MyConanFile
from conans import ConanFile, tools
from conan.tools.cmake import CMakeToolchain, CMake
from conan.tools.files import collect_libs, copy, rmdir


class LibxlConan(MyConanFile):
    name = "libxl"
    version = "4.0.3"
    homepage = "https://www.libxl.com/"
    description = "LibXL is a library that can read and write Excel files. "
    license = "Commercial"

    settings = "os", "arch", "compiler", "build_type"
    options = {
        "shared": [True, False],
        "fPIC": [True, False],
    }
    default_options = {
        "shared": True,
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
        tools.unzip(srcFile, destination=self.source_folder, strip_root=False)

    def generate(self):
        tc = CMakeToolchain(self)
        tc.variables["LIBXL_SHARED"] = "1" if self.options.shared else "0"
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package_id(self):
        del self.info.settings.build_type

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.set_property("cmake_file_name", "libxl")
        self.cpp_info.set_property("cmake_target_name", "libxl::libxl")
        self.cpp_info.set_property("pkg_config_name", "libxl")
        self.cpp_info.libs = collect_libs(self, folder="lib")
