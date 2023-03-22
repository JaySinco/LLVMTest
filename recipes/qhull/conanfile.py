import sys, os
from myconanfile import MyConanFile
from conans import ConanFile, tools
from conan.tools.cmake import CMakeToolchain, CMake
from conan.tools.files import collect_libs, copy, rmdir
from conan.tools.microsoft import is_msvc


class QhullConan(MyConanFile):
    name = "qhull"
    version = "8.0.2"
    homepage = "http://www.qhull.org"
    description = "Qhull computes the convex hull, Delaunay triangulation, Voronoi diagram, halfspace intersection about a point, furthest-site Delaunay triangulation, and furthest-site Voronoi diagram"
    license = "Qhull"

    settings = "os", "arch", "compiler", "build_type"
    options = {
        "shared": [True, False],
        "fPIC": [True, False],
        "reentrant": [True, False],
    }
    default_options = {
        "shared": False,
        "fPIC": True,
        "reentrant": True,
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
        self._patch_sources(self._dirname(__file__), [
            "0001-fix-cmake-minimum-required-location.patch",
        ])

    def generate(self):
        tc = CMakeToolchain(self)
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package_id(self):
        del self.info.options.reentrant

    def package(self):
        copy(self, "COPYING.txt", dst=os.path.join(
            self.package_folder, "licenses"), src=self.source_folder)
        cmake = CMake(self)
        cmake.install()
        rmdir(self, os.path.join(self.package_folder, "doc"))
        rmdir(self, os.path.join(self.package_folder, "man"))
        rmdir(self, os.path.join(self.package_folder, "lib", "cmake"))
        rmdir(self, os.path.join(self.package_folder, "lib", "pkgconfig"))
        rmdir(self, os.path.join(self.package_folder, "share"))

    def package_info(self):
        self.cpp_info.set_property("cmake_file_name", "qhull")
        self.cpp_info.set_property("cmake_target_name", f"qhull::{self._qhull_cmake_name}")
        self.cpp_info.set_property("cmake_target_aliases", [self._qhull_cmake_name])
        self.cpp_info.set_property("pkg_config_name", self._qhull_pkgconfig_name)
        self.cpp_info.includedirs = [
            "include",
            "include/libqhull_r",
        ]
        self.cpp_info.libs = [self._qhull_lib_name]
        if self.settings.os in ["Linux", "FreeBSD"]:
            self.cpp_info.system_libs.append("m")
        if is_msvc(self) and self.options.shared:
            self.cpp_info.defines.extend(["qh_dllimport"])

    @property
    def _qhull_cmake_name(self):
        name = ""
        if self.options.reentrant:
            name = "qhull_r" if self.options.shared else "qhullstatic_r"
        else:
            name = "libqhull" if self.options.shared else "qhullstatic"
        return name

    @property
    def _qhull_pkgconfig_name(self):
        name = "qhull"
        if not self.options.shared:
            name += "static"
        if self.options.reentrant:
            name += "_r"
        return name

    @property
    def _qhull_lib_name(self):
        name = "qhull"
        if not self.options.shared:
            name += "static"
        if self.settings.build_type == "Debug" or self.options.reentrant:
            name += "_"
            if self.options.reentrant:
                name += "r"
            if self.settings.build_type == "Debug":
                name += "d"
        return name
