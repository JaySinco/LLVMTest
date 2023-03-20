import sys, os
from myconanfile import MyConanFile
from conans import ConanFile, tools
from conan.tools.cmake import CMakeToolchain, CMake
from conan.tools.files import collect_libs, copy, rmdir


class Bzip2Conan(MyConanFile):
    name = "bzip2"
    version = "1.0.8"
    homepage = "http://www.bzip.org"
    description = "bzip2 is a free and open-source file compression program that uses the Burrows Wheeler algorithm."
    license = "bzip2"

    settings = "os", "arch", "compiler", "build_type"
    options = {
        "shared": [True, False],
        "fPIC": [True, False],
        "build_executable": [True, False],
    }
    default_options = {
        "shared": False,
        "fPIC": True,
        "build_executable": True,
    }

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC
        self.license = "bzip2-{}".format(self.version)

    def configure(self):
        if self.options.shared:
            del self.options.fPIC

    def source(self):
        srcFile = self._src_abspath(f"{self.name}-{self.version}.tar.gz")
        tools.unzip(srcFile, destination=self.source_folder, strip_root=True)
        copy(self, "CMakeLists.txt", dst=self.source_folder, src=self._dirname(__file__))

    def generate(self):
        tc = CMakeToolchain(self)
        tc.variables["BZ2_VERSION_STRING"] = self.version
        tc.variables["BZ2_VERSION_MAJOR"] = str(self.version).split(".")[0]
        tc.variables["BZ2_BUILD_EXE"] = self.options.build_executable
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

    def package_info(self):
        self.cpp_info.set_property("cmake_file_name", "BZip2")
        self.cpp_info.set_property("cmake_target_name", "BZip2::BZip2")
        self.cpp_info.libs = collect_libs(self, folder="lib")
