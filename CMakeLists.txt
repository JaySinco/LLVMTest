cmake_minimum_required(VERSION 3.19)

set(VCPKG_TARGET_TRIPLET "x64-windows")
set(VCPKG_HOST_TRIPLET "x64-windows")
set(VCPKG_MANIFEST_MODE ON)

project(Prototyping)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")

add_compile_options(-Wno-ignored-attributes)
add_compile_definitions(UNICODE)

add_subdirectory(src)
