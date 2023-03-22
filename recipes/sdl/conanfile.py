import sys, os
from myconanfile import MyConanFile
from conans import ConanFile, tools
from conan.tools.cmake import CMakeToolchain, CMake
from conan.tools.files import collect_libs, copy, rmdir, rm
from conan.tools.microsoft import msvc_runtime_flag, is_msvc


class SDLConan(MyConanFile):
    name = "sdl"
    version = "2.24.1"
    homepage = "https://www.libsdl.org"
    description = "Access to audio, keyboard, mouse, joystick, and graphics hardware via OpenGL, Direct3D and Vulkan"
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
        tc.variables["SDL2_DISABLE_INSTALL"] = False
        if self.settings.os != "Windows" and not self.options.shared:
            tc.variables["SDL_STATIC_PIC"] = self.options.fPIC
        if is_msvc(self) and not self.options.shared:
            tc.variables["HAVE_LIBC"] = True
        tc.variables["SDL_SHARED"] = self.options.shared
        tc.variables["SDL_STATIC"] = not self.options.shared
        if self.settings.os == "Linux":
            tc.variables["SDL_VIDEO_DRIVER_X11_SUPPORTS_GENERIC_EVENTS"] = 1
        elif self.settings.os == "Windows":
            tc.variables["SDL_DIRECTX"] = True
        tc.variables["SDL2_DISABLE_SDL2MAIN"] = True
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        copy(self, "LICENSE.txt", dst=os.path.join(
            self.package_folder, "licenses"), src=self.source_folder)
        cmake = CMake(self)
        cmake.install()
        rm(self, "sdl2-config", os.path.join(self.package_folder, "bin"))
        rmdir(self, os.path.join(self.package_folder, "cmake"))
        rmdir(self, os.path.join(self.package_folder, "lib", "cmake"))
        rmdir(self, os.path.join(self.package_folder, "lib", "pkgconfig"))
        rmdir(self, os.path.join(self.package_folder, "libdata"))
        rmdir(self, os.path.join(self.package_folder, "share"))

    def package_info(self):
        self.cpp_info.set_property("cmake_file_name", "SDL2")
        self.cpp_info.set_property("cmake_target_name", "SDL2::SDL2")
        self.cpp_info.set_property("pkg_config_name", "sdl2")
        postfix = "d" if self.settings.os != "Android" and self.settings.build_type == "Debug" else ""
        lib_postfix = postfix
        if is_msvc(self) and not self.options.shared:
            lib_postfix = "-static" + postfix
        self.cpp_info.libs = ["SDL2" + lib_postfix]
        if self.settings.os == "Windows":
            self.cpp_info.system_libs.extend( ["user32", "gdi32", "winmm", "imm32", "ole32", "oleaut32", "version", "uuid", "advapi32", "setupapi", "shell32"])
        elif self.settings.os in ["Linux", "FreeBSD"]:
            self.cpp_info.system_libs.extend(["dl", "rt", "pthread"])
