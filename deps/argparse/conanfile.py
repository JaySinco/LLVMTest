import sys, os
from myconanfile import MyConanFile
from conans import ConanFile, tools
from conan.tools.files import copy


class ArgparseConan(MyConanFile):
    name = "argparse"
    version = "2.9"
    homepage = "https://github.com/p-ranav/argparse"
    description = "Argument Parser for Modern C++"
    license = "MIT"

    settings = "os", "arch", "compiler", "build_type"

    def source(self):
        srcFile = self._src_abspath(f"{self.name}-{self.version}.tar.gz")
        tools.unzip(srcFile, destination=self.source_folder, strip_root=True)

    def package_id(self):
        self.info.clear()

    def package(self):
        copy(self, "LICENSE", dst=os.path.join(
            self.package_folder, "licenses"), src=self.source_folder)
        copy(self, "*.hpp", dst=os.path.join(self.package_folder, "include"),
             src=os.path.join(self.source_folder, "include"))

    def package_info(self):
        self.cpp_info.set_property("cmake_file_name", "argparse")
        self.cpp_info.set_property("cmake_target_name", "argparse::argparse")
        self.cpp_info.set_property("pkg_config_name", "argparse")
