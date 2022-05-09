cmake_minimum_required(VERSION 3.16)

project(Prototyping C CXX)

set(CMAKE_CXX_STANDARD 17)

if(UNIX)
    set(CMAKE_INSTALL_RPATH "$ORIGIN")
    set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
elseif(WIN32)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
    add_compile_options(/utf-8)
endif()

set(DEPS_PREFIX ${CMAKE_SOURCE_DIR}/deps/${TARGET_OS})
add_compile_definitions(DEPS_PREFIX="\"\"${DEPS_PREFIX}\"\"")

add_subdirectory(deps)
add_subdirectory(src)
