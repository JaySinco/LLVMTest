import sys, os
from myconanfile import MyConanFile
from conans import ConanFile, tools
from conan.tools.microsoft import is_msvc
from conan.tools.scm import Version
from conan.tools.files import collect_libs, copy, rmdir, chdir, rm
from conans import MSBuild
from conan.tools.gnu import Autotools, AutotoolsToolchain


class LibUSBConan(MyConanFile):
    name = "libusb"
    version = "1.0.26"
    homepage = "https://github.com/libusb/libusb"
    description = "A cross-platform library to access USB devices"
    license = "LGPL-2.1"

    settings = "os", "arch", "compiler", "build_type"
    options = {
        "shared": [True, False],
        "fPIC": [True, False],
        "enable_udev": [True, False],
    }
    default_options = {
        "shared": False,
        "fPIC": True,
        "enable_udev": True,
    }

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC
        if self.settings.os not in ["Linux", "Android"]:
            del self.options.enable_udev

    def configure(self):
        if self.options.shared:
            del self.options.fPIC

    def source(self):
        srcFile = self._src_abspath(f"{self.name}-{self.version}.tar.gz")
        tools.unzip(srcFile, destination=self.source_folder, strip_root=True)

    def generate(self):
        if self.settings.os in ["Linux", "Android"]:
            tc = AutotoolsToolchain(self)
            tc.configure_args.extend([
                "--enable-shared" if self.options.shared else "--disable-shared",
                "--enable-static" if not self.options.shared else "--disable-static",
                "--enable-udev" if self.options.enable_udev else "--disable-udev",
            ])
            tc.generate()

    def build(self):
        if is_msvc(self):
            with chdir(self, self.source_folder):
                solution_msvc_year = 2019
                solution_file = os.path.join("msvc", "libusb_{}.sln".format(solution_msvc_year))
                platforms = {"x86":"Win32"}
                msbuild = MSBuild(self)
                build_type = "Debug" if self.settings.build_type == "Debug" else "Release"
                msbuild.build(solution_file, platforms=platforms, upgrade_project=False, build_type=build_type)
        else:
            autotools = Autotools(self)
            autotools.autoreconf()
            autotools.configure()
            autotools.make()

    def package(self):
        copy(self, "COPYING", dst=os.path.join(self.package_folder, "licenses"), src=self.source_folder)
        if is_msvc(self):
            copy(self, pattern="libusb.h", dst=os.path.join(self.package_folder, "include", "libusb-1.0"),
                src=os.path.join(self.source_folder, "libusb"), keep_path=False)
            arch = "x64" if self.settings.arch == "x86_64" else "Win32"
            source_dir = os.path.join(self.source_folder, arch, str(self.settings.build_type), "dll" if self.options.shared else "lib")
            if self.options.shared:
                copy(self, pattern="libusb-1.0.dll", dst=os.path.join(self.package_folder,"bin"), src=source_dir, keep_path=False)
                copy(self, pattern="libusb-1.0.lib", dst=os.path.join(self.package_folder,"lib"), src=source_dir, keep_path=False)
            else:
                copy(self, pattern="libusb-1.0.lib", dst=os.path.join(self.package_folder,"lib"), src=source_dir, keep_path=False)
        else:
            autotools = Autotools(self)
            autotools.install()
            rmdir(self, os.path.join(self.package_folder, "lib", "pkgconfig"))
            rm(self, "*.la", os.path.join(self.package_folder, "lib"))

    def package_info(self):
        self.cpp_info.set_property("cmake_file_name", "libusb")
        self.cpp_info.set_property("cmake_target_name", "libusb::libusb")
        self.cpp_info.set_property("pkg_config_name", "libusb-1.0")
        self.cpp_info.includedirs = [
            "include",
            "include/libusb-1.0",
        ]
        self.cpp_info.libs = collect_libs(self, folder="lib")
        if self.settings.os == "Windows":
            self.cpp_info.system_libs.extend(["advapi32"])
        elif self.settings.os in ["Linux", "FreeBSD"]:
            self.cpp_info.system_libs.extend(["pthread"])
