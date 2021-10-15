cmake_minimum_required(VERSION 3.19)

set(VCPKG_TARGET_TRIPLET "x64-windows")
set(VCPKG_HOST_TRIPLET "x64-windows")
set(VCPKG_MANIFEST_MODE ON)
set(VCPKG_APPLOCAL_DEPS ON)
set(VCPKG_BOOTSTRAP_OPTIONS "-verbose -disableMetrics")

message(STATUS "Using toolchain file: ${CMAKE_TOOLCHAIN_FILE}")

project(Prototyping)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")

add_compile_options(-Wno-ignored-attributes)
add_compile_definitions(UNICODE)

add_subdirectory(src)
