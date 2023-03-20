import sys, os
from myconanfile import MyConanFile
from conans import ConanFile, tools
from conan.tools.cmake import CMakeToolchain, CMake, CMakeDeps
from conan.tools.microsoft import msvc_runtime_flag, is_msvc
from conan.tools.files import collect_libs, copy, rmdir


class LibeventConan(MyConanFile):
    name = "libevent"
    version = "2.1.12"
    homepage = "https://github.com/libevent/libevent"
    description = "libevent - an event notification library"
    license = "BSD-3-Clause"

    settings = "os", "arch", "compiler", "build_type"
    options = {
        "shared": [True, False],
        "fPIC": [True, False],
        "with_openssl": [True, False],
        "disable_threads": [True, False],
    }
    default_options = {
        "shared": False,
        "fPIC": True,
        "with_openssl": True,
        "disable_threads": False,
    }

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def configure(self):
        if self.options.shared:
            del self.options.fPIC

    def requirements(self):
        if self.options.with_openssl:
            self.requires(self._ref_pkg("openssl/1.1.1q"))

    def source(self):
        srcFile = self._src_abspath(f"{self.name}-{self.version}.tar.gz")
        tools.unzip(srcFile, destination=self.source_folder, strip_root=True)
        self._patch_sources(self._dirname(__file__), [
            "0001-fix-cmake-openssl-uppercase.patch",
            "0002-fix-cmake-install-error.patch",
        ])

    def generate(self):
        tc = CMakeToolchain(self)
        tc.cache_variables["EVENT__LIBRARY_TYPE"] = "SHARED" if self.options.shared else "STATIC"
        tc.variables["EVENT__DISABLE_DEBUG_MODE"] = self.settings.build_type == "Release"
        tc.variables["EVENT__DISABLE_OPENSSL"] = not self.options.with_openssl
        tc.variables["EVENT__DISABLE_THREAD_SUPPORT"] = self.options.disable_threads
        tc.variables["EVENT__DISABLE_BENCHMARK"] = True
        tc.variables["EVENT__DISABLE_TESTS"] = True
        tc.variables["EVENT__DISABLE_REGRESS"] = True
        tc.variables["EVENT__DISABLE_SAMPLES"] = True
        if is_msvc(self):
            tc.variables["EVENT__MSVC_STATIC_RUNTIME"] = "MT" in msvc_runtime_flag(self)
        tc.generate()
        cmake_deps = CMakeDeps(self)
        cmake_deps.generate()

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
        self.cpp_info.set_property("cmake_file_name", "Libevent")
        self.cpp_info.set_property("pkg_config_name", "libevent")
        # core
        self.cpp_info.components["core"].set_property("cmake_target_name", "libevent::core")
        self.cpp_info.components["core"].set_property("pkg_config_name", "libevent_core")
        self.cpp_info.components["core"].libs = ["event_core"]
        if self.settings.os in ["Linux", "FreeBSD"] and not self.options.disable_threads:
            self.cpp_info.components["core"].system_libs = ["pthread"]
        if self.settings.os == "Windows":
            self.cpp_info.components["core"].system_libs = ["ws2_32", "advapi32", "iphlpapi"]
        # extra
        self.cpp_info.components["extra"].set_property("cmake_target_name", "libevent::extra")
        self.cpp_info.components["extra"].set_property("pkg_config_name", "libevent_extra")
        self.cpp_info.components["extra"].libs = ["event_extra"]
        if self.settings.os == "Windows":
            self.cpp_info.components["extra"].system_libs = ["shell32"]
        self.cpp_info.components["extra"].requires = ["core"]
        # openssl
        if self.options.with_openssl:
            self.cpp_info.components["openssl"].set_property("cmake_target_name", "libevent::openssl")
            self.cpp_info.components["openssl"].set_property("pkg_config_name", "libevent_openssl")
            self.cpp_info.components["openssl"].libs = ["event_openssl"]
            self.cpp_info.components["openssl"].requires = ["core", "openssl::openssl"]
        # pthreads
        if self.settings.os != "Windows" and not self.options.disable_threads:
            self.cpp_info.components["pthreads"].set_property("cmake_target_name", "libevent::pthreads")
            self.cpp_info.components["pthreads"].set_property("pkg_config_name", "libevent_pthreads")
            self.cpp_info.components["pthreads"].libs = ["event_pthreads"]
            self.cpp_info.components["pthreads"].requires = ["core"]
