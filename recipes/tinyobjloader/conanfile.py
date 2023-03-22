import sys, os
from myconanfile import MyConanFile
from conans import ConanFile, tools
from conan.tools.cmake import CMakeToolchain, CMake
from conan.tools.files import collect_libs, copy, rmdir


class TinyObjLoaderConan(MyConanFile):
    name = "tinyobjloader"
    version = "v2020.02.28"
    homepage = "https://github.com/tinyobjloader/tinyobjloader"
    description = "Tiny but powerful single file wavefront obj loader"
    license = "MIT"

    settings = "os", "arch", "compiler", "build_type"
    options = {
        "shared": [True, False],
        "fPIC": [True, False],
        "double": [True, False],
    }
    default_options = {
        "shared": False,
        "fPIC": True,
        "double": False,
    }

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def configure(self):
        if self.options.shared:
            del self.options.fPIC

    def source(self):
        srcFile = self._src_abspath(f"{self.name}-{self.version}.zip")
        tools.unzip(srcFile, destination=self.source_folder, strip_root=True)
        self._patch_sources(self._dirname(__file__), [
            "0001-fix-cmake-minimum-required-location.patch",
        ])

    def generate(self):
        tc = CMakeToolchain(self)
        tc.variables["TINYOBJLOADER_USE_DOUBLE"] = self.options.double
        tc.variables["TINYOBJLOADER_BUILD_TEST_LOADER"] = False
        tc.variables["TINYOBJLOADER_COMPILATION_SHARED"] = self.options.shared
        tc.variables["TINYOBJLOADER_BUILD_OBJ_STICHER"] = False
        tc.variables["CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS"] = True
        tc.variables["CMAKE_INSTALL_DOCDIR"] = "licenses"
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()
        rmdir(self, os.path.join(self.package_folder, "lib", "pkgconfig"))
        rmdir(self, os.path.join(self.package_folder, "lib", "tinyobjloader"))

    def package_info(self):
        suffix = "_double" if self.options.double else ""
        name = "tinyobjloader{}".format(suffix)
        self.cpp_info.set_property("cmake_file_name", "tinyobjloader")
        self.cpp_info.set_property("cmake_target_name", name)
        self.cpp_info.set_property("pkg_config_name", name)
        self.cpp_info.libs = [name]
        if self.options.double:
            self.cpp_info.defines.append("TINYOBJLOADER_USE_DOUBLE")
