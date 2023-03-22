import sys, os
from myconanfile import MyConanFile
from conans import ConanFile, tools
from conan.tools.cmake import CMakeToolchain, CMake
from conan.tools.files import collect_libs, copy, rmdir


class SnappyConan(MyConanFile):
    name = "snappy"
    version = "1.1.9"
    homepage = "https://github.com/google/snappy"
    description = "A fast compressor/decompressor"
    license = "BSD-3-Clause"

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
        self._patch_sources(self._dirname(__file__), [
            "0001-fix-inlining-failure.patch",
        ])

    def generate(self):
        tc = CMakeToolchain(self)
        tc.variables["SNAPPY_BUILD_TESTS"] = False
        tc.variables["SNAPPY_FUZZING_BUILD"] = False
        tc.variables["SNAPPY_REQUIRE_AVX"] = False
        tc.variables["SNAPPY_REQUIRE_AVX2"] = False
        tc.variables["SNAPPY_INSTALL"] = True
        tc.variables["SNAPPY_BUILD_BENCHMARKS"] = False
        tc.cache_variables["CMAKE_POLICY_DEFAULT_CMP0077"] = "NEW"
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        copy(self, "COPYING", dst=os.path.join(
            self.package_folder, "licenses"), src=self.source_folder)
        cmake = CMake(self)
        cmake.install()
        rmdir(self, os.path.join(self.package_folder, "lib", "cmake"))

    def package_info(self):
        self.cpp_info.set_property("cmake_file_name", "Snappy")
        self.cpp_info.set_property("cmake_target_name", "Snappy::snappy")
        self.cpp_info.set_property("pkg_config_name", "snappy")
        self.cpp_info.libs = collect_libs(self, folder="lib")
