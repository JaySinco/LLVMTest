cmake_minimum_required(VERSION 3.19)

set(VCPKG_APPLOCAL_DEPS ON)
set(VCPKG_VERBOSE OFF)
set(VCPKG_HOST_TRIPLET "x64-windows")
set(VCPKG_TARGET_TRIPLET "x64-windows")
set(VCPKG_BOOTSTRAP_OPTIONS "-verbose;-disableMetrics")
set(VCPKG_INSTALL_OPTIONS "--no-binarycaching")
set(VCPKG_MANIFEST_DIR ${CMAKE_SOURCE_DIR}/.vscode)
set(VCPKG_INSTALLED_DIR ${VCPKG_ROOT_DIR}/installed)

set(CMAKE_TOOLCHAIN_FILE ${VCPKG_ROOT_DIR}/scripts/buildsystems/vcpkg.cmake)
message(STATUS "Using toolchain file: ${CMAKE_TOOLCHAIN_FILE}")

project(Prototyping C CXX ASM_NASM)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
set(PROTO_VCPKG_PREFIX ${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET})

add_subdirectory(deps)
add_subdirectory(src)
