import sys, os
from myconanfile import MyConanFile
from conans import ConanFile, tools
from conan.tools.cmake import CMakeToolchain, CMake
from conan.tools.files import collect_libs, copy, rmdir
from conan.tools.microsoft import msvc_runtime_flag, is_msvc


class LZ4Conan(MyConanFile):
    name = "lz4"
    version = "1.9.4"
    homepage = "https://github.com/lz4/lz4"
    description = "Extremely Fast Compression algorithm"
    license = ("BSD-2-Clause", "BSD-3-Clause")

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

    def generate(self):
        tc = CMakeToolchain(self)
        tc.variables["LZ4_BUILD_CLI"] = False
        tc.variables["LZ4_BUILD_LEGACY_LZ4C"] = False
        tc.variables["LZ4_BUNDLED_MODE"] = False
        tc.variables["LZ4_POSITION_INDEPENDENT_LIB"] = self.options.get_safe("fPIC", True)
        tc.cache_variables["CMAKE_POLICY_DEFAULT_CMP0042"] = "NEW"
        tc.cache_variables["CMAKE_POLICY_DEFAULT_CMP0077"] = "NEW"
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure(build_script_folder=self._cmakelists_folder)
        cmake.build()

    def package(self):
        copy(self, "LICENSE", dst=os.path.join(
            self.package_folder, "licenses"), src=self.source_folder)
        cmake = CMake(self)
        cmake.install()
        rmdir(self, os.path.join(self.package_folder, "lib", "pkgconfig"))
        rmdir(self, os.path.join(self.package_folder, "lib", "cmake"))
        rmdir(self, os.path.join(self.package_folder, "share"))

    def package_info(self):
        self.cpp_info.set_property("cmake_file_name", "lz4")
        self.cpp_info.set_property("cmake_target_name", f'LZ4::lz4{"" if self.options.shared else "_static"}')
        self.cpp_info.set_property("pkg_config_name", "liblz4")
        self.cpp_info.libs = collect_libs(self, folder="lib")
        if is_msvc(self) and self.options.shared:
            self.cpp_info.defines.append("LZ4_DLL_IMPORT=1")

    @property
    def _cmakelists_folder(self):
        subfolder = os.path.join("build", "cmake")
        return os.path.join(self.source_folder, subfolder)