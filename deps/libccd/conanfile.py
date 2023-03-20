import sys, os
from myconanfile import MyConanFile
from conans import ConanFile, tools
from conan.tools.cmake import CMakeToolchain, CMake
from conan.tools.files import collect_libs, copy, rmdir


class LibccdConan(MyConanFile):
    name = "libccd"
    version = "2.1"
    homepage = "https://github.com/danfis/libccd"
    description = "Library for collision detection between two convex shapes"
    license = "BSD-3-Clause"

    settings = "os", "arch", "compiler", "build_type"
    options = {
        "shared": [True, False],
        "fPIC": [True, False],
        "enable_double_precision": [True, False],
    }
    default_options = {
        "shared": False,
        "fPIC": True,
        "enable_double_precision": True,
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

    def generate(self):
        tc = CMakeToolchain(self)
        tc.variables["BUILD_DOCUMENTATION"] = False
        tc.variables["ENABLE_DOUBLE_PRECISION"] = self.options.enable_double_precision
        tc.variables["CCD_HIDE_ALL_SYMBOLS"] = not self.options.shared
        tc.cache_variables["CMAKE_POLICY_DEFAULT_CMP0077"] = "NEW"
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        copy(self, "BSD-LICENSE", dst=os.path.join(
            self.package_folder, "licenses"), src=self.source_folder)
        cmake = CMake(self)
        cmake.install()
        rmdir(self, os.path.join(self.package_folder, "lib", "ccd"))
        rmdir(self, os.path.join(self.package_folder, "lib", "pkgconfig"))
        rmdir(self, os.path.join(self.package_folder, "share"))

    def package_info(self):
        self.cpp_info.set_property("cmake_file_name", "ccd")
        self.cpp_info.set_property("cmake_target_name", "ccd")
        self.cpp_info.set_property("pkg_config_name", "ccd")
        self.cpp_info.libs = collect_libs(self, folder="lib")
        if not self.options.shared:
            self.cpp_info.defines.append("CCD_STATIC_DEFINE")
        if self.settings.os in ["Linux", "FreeBSD"]:
            self.cpp_info.system_libs.append("m")
