import sys, os
from myconanfile import MyConanFile
from conans import ConanFile, tools
from conan.tools.cmake import CMakeToolchain, CMake, CMakeDeps
from conan.tools.files import collect_libs, copy, rmdir
from conan.tools.microsoft import msvc_runtime_flag, is_msvc


class RaylibConan(MyConanFile):
    name = "raylib"
    version = "4.2.0"
    homepage = "https://www.raylib.com/"
    description = "Raylib is a simple and easy-to-use library to enjoy videogames programming"
    license = "Zlib"

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
        tc.variables["BUILD_EXAMPLES"] = True
        tc.variables["USE_EXTERNAL_GLFW"] = False
        tc.variables["OPENGL_VERSION"] = "4.3"
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
        copy(self, "glad.h",
            dst=os.path.join(self.package_folder, "include"),
            src=os.path.join(self.source_folder, "src", "external"))

    def package_info(self):
        self.cpp_info.set_property("cmake_file_name", "raylib")
        self.cpp_info.set_property("cmake_target_name", "raylib")
        self.cpp_info.set_property("pkg_config_name", "raylib")
        self.cpp_info.libs = ["raylib"]
        if is_msvc(self) and self.options.shared:
            self.cpp_info.defines.append("USE_LIBTYPE_SHARED")
        if self.settings.os in ["Linux", "FreeBSD"]:
            self.cpp_info.system_libs.extend(["m", "pthread", "dl"])
        elif self.settings.os == "Windows":
            self.cpp_info.system_libs.append("winmm")
