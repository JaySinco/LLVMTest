cmake_minimum_required(VERSION 3.16)

project(Prototyping C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INSTALL_RPATH "$ORIGIN")
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)

set(DEPS_PREFIX ${CMAKE_SOURCE_DIR}/deps/linux)
add_compile_definitions(DEPS_PREFIX="\"\"${DEPS_PREFIX}\"\"")

add_subdirectory(deps)
add_subdirectory(src)
