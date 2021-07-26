cmake_minimum_required(VERSION 3.16)

project(ShaderTest)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")

add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

include_directories(AFTER
    ${CMAKE_SOURCE_DIR}/src/
    ${CMAKE_SOURCE_DIR}/deps/gflags/include/
    ${CMAKE_SOURCE_DIR}/deps/glog/include/
    ${CMAKE_SOURCE_DIR}/deps/fmt/include/
    ${CMAKE_SOURCE_DIR}/deps/boost/include/boost-1_76/
)

set(dep_LIB
    ${CMAKE_SOURCE_DIR}/deps/gflags/lib/gflags_static.lib
    ${CMAKE_SOURCE_DIR}/deps/glog/lib/glog.lib
    ${CMAKE_SOURCE_DIR}/deps/fmt/lib/fmt.lib
)

add_executable(demo
    ${CMAKE_SOURCE_DIR}/src/utils.cpp
    ${CMAKE_SOURCE_DIR}/src/parser.cpp
    ${CMAKE_SOURCE_DIR}/src/driver.cpp
)
set_target_properties(demo PROPERTIES LINK_FLAGS "/MANIFEST:NO")
target_compile_definitions(demo PRIVATE UNICODE)
target_link_libraries(demo shlwapi.lib ${dep_LIB})
