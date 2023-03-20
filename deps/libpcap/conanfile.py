import sys, os
from myconanfile import MyConanFile
from conans import ConanFile, tools
from conan.tools.microsoft import is_msvc
from conan.tools.scm import Version
from conan.tools.files import collect_libs, copy, rmdir, chdir, rm
from conans import MSBuild
from conan.tools.gnu import Autotools, AutotoolsToolchain, AutotoolsDeps
from conan.errors import ConanInvalidConfiguration


class LibPcapConan(MyConanFile):
    name = "libpcap"
    version = "1.10.1"
    homepage = "https://github.com/the-tcpdump-group/libpcap"
    description = "libpcap is an API for capturing network traffic"
    license = "BSD-3-Clause"

    settings = "os", "arch", "compiler", "build_type"
    options = {
        "shared": [True, False],
        "fPIC": [True, False],
        "enable_libusb": [True, False],
    }
    default_options = {
        "shared": False,
        "fPIC": True,
        "enable_libusb": True,
    }

    def config_options(self):
        pass

    def configure(self):
        if self.options.shared:
            del self.options.fPIC

    def validate(self):
        if self.info.settings.os != "Linux":
            raise ConanInvalidConfiguration(f"{self.ref} requires Linux")

    def requirements(self):
        if self.options.get_safe("enable_libusb"):
            self.requires(self._ref_pkg("libusb/1.0.26"))

    def source(self):
        srcFile = self._src_abspath(f"{self.name}-{self.version}.tar.gz")
        tools.unzip(srcFile, destination=self.source_folder, strip_root=True)

    def generate(self):
        tc = AutotoolsToolchain(self)
        yes_no = lambda v: "yes" if v else "no"
        tc.configure_args.extend([
            "--enable-shared={}".format(yes_no(self.options.shared)),
            "--enable-static={}".format(yes_no(not self.options.shared)),
            "--enable-usb={}".format(yes_no(self.options.get_safe("enable_libusb"))),
            "--disable-universal",
            "--without-libnl",
            "--disable-bluetooth",
            "--disable-packet-ring",
            "--disable-dbus",
            "--disable-rdma",
        ])
        tc.generate()
        deps = AutotoolsDeps(self)
        deps.generate()

    def build(self):
        autotools = Autotools(self)
        autotools.configure()
        autotools.make()

    def package(self):
        copy(self, "LICENSE", dst=os.path.join(self.package_folder, "licenses"), src=self.source_folder)
        autotools = Autotools(self)
        autotools.install()
        rmdir(self, os.path.join(self.package_folder, "lib", "pkgconfig"))
        rmdir(self, os.path.join(self.package_folder, "share"))
        if self.options.shared:
            tools.remove_files_by_mask(os.path.join(self.package_folder, "lib"), "*.a")

    def package_info(self):
        self.cpp_info.set_property("cmake_file_name", "libpcap")
        self.cpp_info.set_property("cmake_target_name", "libpcap::libpcap")
        self.cpp_info.set_property("pkg_config_name", "libpcap")
        self.cpp_info.libs = ["pcap"]
