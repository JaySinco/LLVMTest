import sys, os
from myconanfile import MyConanFile
from conans import ConanFile, tools
from conan.tools.cmake import CMakeToolchain, CMake, CMakeDeps
from conan.tools.files import collect_libs, copy, rmdir


class MujocoConan(MyConanFile):
    name = "mujoco"
    version = "2.2.2"
    homepage = "https://github.com/deepmind/mujoco"
    description = "Multi-Joint dynamics with Contact. A general purpose physics simulator"
    license = "Apache-2.0"

    settings = "os", "arch", "compiler", "build_type"
    options = {
        "shared": [True, False],
        "fPIC": [True, False],
    }
    default_options = {
        "shared": True,
        "fPIC": True,
    }

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def configure(self):
        if self.options.shared:
            del self.options.fPIC

    def requirements(self):
        self.requires(self._ref_pkg("libccd/2.1"))
        self.requires(self._ref_pkg("qhull/8.0.2"))
        self.requires(self._ref_pkg("lodepng/v2022.07.18"))
        self.requires(self._ref_pkg("tinyobjloader/v2020.02.28"))
        self.requires(self._ref_pkg("tinyxml2/9.0.0"))
        self.requires(self._ref_pkg("glfw/3.3.7"))

    def source(self):
        srcFile = self._src_abspath(f"{self.name}-{self.version}.tar.gz")
        tools.unzip(srcFile, destination=self.source_folder, strip_root=True)
        self._patch_sources(self._dirname(__file__), [
            "0001-fix-cmake-findorfetch-dependencies.patch",
            "0002-fix-simulate-findorfetch-glfw.patch",
        ])

    def generate(self):
        tc = CMakeToolchain(self)
        tc.variables["MUJOCO_BUILD_EXAMPLES"] = True
        tc.variables["MUJOCO_BUILD_SIMULATE"] = True
        tc.variables["MUJOCO_BUILD_TESTS"] = False
        tc.variables["MUJOCO_TEST_PYTHON_UTIL"] = False
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
        rmdir(self, os.path.join(self.package_folder, "lib", "pkgconfig"))
        rmdir(self, os.path.join(self.package_folder, "lib", "cmake"))

    def package_info(self):
        self.cpp_info.set_property("cmake_file_name", "mujoco")
        self.cpp_info.set_property("cmake_target_name", "mujoco::mujoco")
        self.cpp_info.set_property("pkg_config_name", "mujoco")
        self.cpp_info.libs = collect_libs(self, folder="lib")
