import sys, os
from myconanfile import MyConanFile
from conans import ConanFile, tools
from conan.tools.cmake import CMakeToolchain, CMake, CMakeDeps
from conan.tools.files import collect_libs, copy, rmdir


class MongooseJsonConan(MyConanFile):
    name = "mongoose"
    version = "7.8"
    homepage = "https://github.com/cesanta/mongoose"
    description = "Embedded Web Server"
    license = "GPL-3.0"

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
        copy(self, "CMakeLists.txt", src=self._dirname(__file__),
            dst=self.source_folder)

    def requirements(self):
        self.requires(self._ref_pkg("openssl/1.1.1q"))

    def generate(self):
        tc = CMakeToolchain(self)
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

    def package_info(self):
        self.cpp_info.set_property("cmake_file_name", "mongoose")
        self.cpp_info.set_property("cmake_target_name", "mongoose")
        self.cpp_info.set_property("pkg_config_name", "mongoose")
        self.cpp_info.libs = ["mongoose"]
        self.cpp_info.defines.append("MG_ENABLE_OPENSSL=1")
