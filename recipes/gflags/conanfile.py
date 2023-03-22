import sys, os
from myconanfile import MyConanFile
from conans import ConanFile, tools
from conan.tools.cmake import CMakeToolchain, CMake
from conan.tools.files import collect_libs, copy, rmdir


class GflagsConan(MyConanFile):
    name = "gflags"
    version = "2.2.2"
    homepage = "https://github.com/gflags/gflags"
    description = "The gflags package contains a C++ library that implements commandline flags processing"
    license = "BSD-3-Clause"

    settings = "os", "arch", "compiler", "build_type"
    options = {
        "shared": [True, False],
        "fPIC": [True, False],
        "nothreads": [True, False],
        "namespace": ["ANY"]
    }
    default_options = {
        "shared": False,
        "fPIC": True,
        "nothreads": False,
        "namespace": "gflags",
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
        tc.variables["BUILD_SHARED_LIBS"] = self.options.shared
        tc.variables["BUILD_STATIC_LIBS"] = not self.options.shared
        tc.variables["BUILD_gflags_LIB"] = not self.options.nothreads
        tc.variables["BUILD_gflags_nothreads_LIB"] = self.options.nothreads
        tc.variables["BUILD_PACKAGING"] = False
        tc.variables["BUILD_TESTING"] = False
        tc.variables["INSTALL_HEADERS"] = True
        tc.variables["INSTALL_SHARED_LIBS"] = self.options.shared
        tc.variables["INSTALL_STATIC_LIBS"] = not self.options.shared
        tc.variables["REGISTER_BUILD_DIR"] = False
        tc.variables["REGISTER_INSTALL_PREFIX"] = False
        tc.variables["GFLAGS_NAMESPACE"] = self.options.namespace
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        copy(self, "COPYING.txt", dst=os.path.join(
            self.package_folder, "licenses"), src=self.source_folder)
        cmake = CMake(self)
        cmake.install()
        rmdir(self, os.path.join(self.package_folder, "lib", "pkgconfig"))
        rmdir(self, os.path.join(self.package_folder, "lib", "cmake"))

    def package_info(self):
        self.cpp_info.set_property("cmake_file_name", "gflags")
        self.cpp_info.set_property("cmake_target_name", "gflags::gflags")
        # self.cpp_info.set_property("cmake_target_aliases", ["gflags"])
        self.cpp_info.set_property("pkg_config_name", "gflags")
        self.cpp_info.libs = collect_libs(self, folder="lib")
        if self.settings.os == "Windows":
            self.cpp_info.system_libs.extend(["shlwapi"])
        elif self.settings.os in ["Linux", "FreeBSD"]:
            self.cpp_info.system_libs.extend(["pthread", "m"])
