import sys, os
from myconanfile import MyConanFile
from conans import ConanFile, tools
from conan.tools.cmake import CMakeToolchain, CMake, CMakeDeps
from conan.tools.files import collect_libs, copy, rmdir


class GlogConan(MyConanFile):
    name = "glog"
    version = "0.6.0"
    homepage = "https://github.com/google/glog/"
    description = "Google logging library"
    license = "BSD-3-Clause"

    settings = "os", "arch", "compiler", "build_type"
    options = {
        "shared": [True, False],
        "fPIC": [True, False],
        "with_gflags": [True, False],
        "with_threads": [True, False],
    }
    default_options = {
        "shared": False,
        "fPIC": True,
        "with_gflags": True,
        "with_threads": True,
    }

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def configure(self):
        if self.options.shared:
            del self.options.fPIC
        if self.options.with_gflags:
            self.options["gflags"].shared = self.options.shared

    def requirements(self):
        if self.options.with_gflags:
            self.requires(self._ref_pkg("gflags/2.2.2"))

    def source(self):
        srcFile = self._src_abspath(f"{self.name}-{self.version}.tar.gz")
        tools.unzip(srcFile, destination=self.source_folder, strip_root=True)

    def generate(self):
        tc = CMakeToolchain(self)
        tc.variables["WITH_GFLAGS"] = self.options.with_gflags
        tc.variables["WITH_THREADS"] = self.options.with_threads
        tc.variables["WITH_PKGCONFIG"] = True
        tc.variables["WITH_SYMBOLIZE"] = True
        tc.variables["WITH_UNWIND"] = True
        tc.variables["BUILD_TESTING"] = False
        tc.generate()
        cmake_deps = CMakeDeps(self)
        cmake_deps.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        copy(self, "COPYING", dst=os.path.join(
            self.package_folder, "licenses"), src=self.source_folder)
        cmake = CMake(self)
        cmake.install()
        rmdir(self, os.path.join(self.package_folder, "lib", "pkgconfig"))
        rmdir(self, os.path.join(self.package_folder, "lib", "cmake"))

    def package_info(self):
        self.cpp_info.set_property("cmake_file_name", "glog")
        self.cpp_info.set_property("cmake_target_name", "glog::glog")
        self.cpp_info.set_property("cmake_target_aliases", ["glog"])
        self.cpp_info.set_property("pkg_config_name", "libglog")
        self.cpp_info.libs = collect_libs(self, folder="lib")
        if self.settings.os == "Windows":
            self.cpp_info.system_libs.extend(["dbghelp"])
            self.cpp_info.defines.append("GLOG_NO_ABBREVIATED_SEVERITIES")
            decl = "__declspec(dllimport)" if self.options.shared else ""
            self.cpp_info.defines.append("GOOGLE_GLOG_DLL_DECL={}".format(decl))
        elif self.settings.os in ["Linux", "FreeBSD"]:
            self.cpp_info.system_libs.extend(["pthread"])
        if self.options.with_gflags and not self.options.shared:
            self.cpp_info.defines.extend(["GFLAGS_DLL_DECLARE_FLAG=", "GFLAGS_DLL_DEFINE_FLAG="])
