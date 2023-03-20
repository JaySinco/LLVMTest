import sys, os
from myconanfile import MyConanFile
from conans import ConanFile, tools
from conan.tools.cmake import CMakeToolchain, CMake, CMakeDeps
from conan.tools.files import collect_libs, copy, rmdir
from conan.tools.microsoft import is_msvc_static_runtime, is_msvc


class GTestConan(MyConanFile):
    name = "gtest"
    version = "1.12.1"
    homepage = "https://github.com/google/googletest"
    description = "Google's C++ test framework"
    license = "BSD-3-Clause"

    settings = "os", "arch", "compiler", "build_type"
    options = {
        "shared": [True, False],
        "fPIC": [True, False],
        "build_gmock": [True, False],
        "hide_symbols": [True, False],
    }
    default_options = {
        "shared": False,
        "fPIC": True,
        "build_gmock": True,
        "hide_symbols": False,
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
        tc.cache_variables["CMAKE_POLICY_DEFAULT_CMP0077"] = "NEW"
        tc.variables["BUILD_GMOCK"] = self.options.build_gmock
        tc.variables["gtest_hide_internal_symbols"] = self.options.hide_symbols
        if is_msvc(self):
            tc.variables["gtest_force_shared_crt"] = not is_msvc_static_runtime(self)
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        copy(self, "LICENSE", dst=os.path.join(
            self.package_folder, "licenses"), src=self.source_folder)
        cmake = CMake(self)
        cmake.install()
        rmdir(self, os.path.join(self.package_folder, "lib", "pkgconfig"))
        rmdir(self, os.path.join(self.package_folder, "lib", "cmake"))

    def package_info(self):
        self.cpp_info.set_property("cmake_file_name", "GTest")

        # gtest
        self.cpp_info.components["libgtest"].set_property("cmake_target_name", "GTest::gtest")
        self.cpp_info.components["libgtest"].set_property("cmake_target_aliases", ["GTest::GTest"])
        self.cpp_info.components["libgtest"].set_property("pkg_config_name", "gtest")
        self.cpp_info.components["libgtest"].libs = ["gtest"]
        if self.settings.os in ["Linux", "FreeBSD"]:
            self.cpp_info.components["libgtest"].system_libs.append("m")
            self.cpp_info.components["libgtest"].system_libs.append("pthread")
        if self.options.shared:
            self.cpp_info.components["libgtest"].defines.append("GTEST_LINKED_AS_SHARED_LIBRARY=1")

        # gtest_main
        self.cpp_info.components["gtest_main"].set_property("cmake_target_name", "GTest::gtest_main")
        self.cpp_info.components["gtest_main"].set_property("cmake_target_aliases", ["GTest::Main"])
        self.cpp_info.components["gtest_main"].set_property("pkg_config_name", "gtest_main")
        self.cpp_info.components["gtest_main"].libs = ["gtest_main"]
        self.cpp_info.components["gtest_main"].requires = ["libgtest"]

        # gmock
        if self.options.build_gmock:
            self.cpp_info.components["gmock"].set_property("cmake_target_name", "GTest::gmock")
            self.cpp_info.components["gmock"].set_property("pkg_config_name", "gmock")
            self.cpp_info.components["gmock"].libs = ["gmock"]
            self.cpp_info.components["gmock"].requires = ["libgtest"]
            self.cpp_info.components["gmock_main"].set_property("cmake_target_name", "GTest::gmock_main")
            self.cpp_info.components["gmock_main"].set_property("pkg_config_name", "gmock_main")
            self.cpp_info.components["gmock_main"].libs = ["gmock_main"]
            self.cpp_info.components["gmock_main"].requires = ["gmock"]
