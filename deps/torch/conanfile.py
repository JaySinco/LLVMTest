import sys, os
from myconanfile import MyConanFile
from conans import ConanFile, tools
from conan.tools.files import rmdir, rm, rename
from conan.tools.microsoft import is_msvc
import glob
import shutil


class TorchConan(MyConanFile):
    name = "torch"
    version = "1.8.2"
    homepage = "https://github.com/pytorch/pytorch"
    description = "Tensors and Dynamic neural networks in Python with strong GPU acceleration"
    license = "BSD"

    settings = "os", "arch", "compiler", "build_type"

    def package_id(self):
        del self.info.settings.compiler
        del self.info.settings.build_type

    def package(self):
        filename = "libtorch-win-shared-with-deps-{}+cu111.zip".format(self.version)
        if self.settings.os == "Linux":
            filename = "libtorch-cxx11-abi-shared-with-deps-{}+cpu.zip".format(self.version)

        srcFile = self._src_abspath(filename)
        tools.unzip(srcFile, destination=self.package_folder, strip_root=True)

        if self.settings.os == "Windows":
            rm(self, "*.exe",  os.path.join(self.package_folder, "bin"))
            files = glob.glob(os.path.join(self.package_folder, "lib", "*.dll"))
            for file in files:
                shutil.move(file, os.path.join(self.package_folder, "bin"))

        rm(self, "build-*",  self.package_folder)
        rmdir(self, os.path.join(self.package_folder, "test"))
        rmdir(self, os.path.join(self.package_folder, "cmake"))
        rmdir(self, os.path.join(self.package_folder, "share"))

    def package_info(self):
        self.cpp_info.set_property("cmake_file_name", "Torch")
        self.cpp_info.set_property("cmake_target_name", "Torch::torch")
        self.cpp_info.set_property("pkg_config_name", "Torch")
        self.cpp_info.includedirs = [
            "include",
            "include/torch/csrc/api/include",
        ]
        if self.settings.os == "Linux":
            self.cpp_info.libs = [
                "libc10.so",
                "libtorch_cpu.so",
            ]
        elif self.settings.os == "Windows":
            self.cpp_info.libs = [
                "c10",
                "torch_cpu",
                "torch_cuda_cpp",
                "torch_cuda_cu",
            ]
        if is_msvc(self):
            self.cpp_info.cxxflags = [
                "/W0",
            ]
