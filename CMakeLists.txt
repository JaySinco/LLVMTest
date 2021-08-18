cmake_minimum_required(VERSION 3.16)

project(ShaderTest)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")

add_compile_definitions(UNICODE)
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

file(GLOB_RECURSE ANTLR4_RUNTIME_SRC
    ${CMAKE_SOURCE_DIR}/deps/antlr4/src/antlr4-4.9.2/runtime/Cpp/runtime/src/*.cpp
)
add_library(antlr4_runtime ${ANTLR4_RUNTIME_SRC})
target_include_directories(antlr4_runtime PUBLIC
    ${CMAKE_SOURCE_DIR}/deps/antlr4/src/antlr4-4.9.2/runtime/Cpp/runtime/src
)
target_compile_definitions(antlr4_runtime PUBLIC
    ANTLR4CPP_STATIC
    _CRT_SECURE_NO_WARNINGS
    _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
)

file(GLOB_RECURSE GEN_CPP_SRC ${CMAKE_SOURCE_DIR}/src/gen-cpp/*.cpp)
add_executable(demo
    ${GEN_CPP_SRC}
    ${CMAKE_SOURCE_DIR}/src/utils.cpp
    ${CMAKE_SOURCE_DIR}/src/jit-compiler.cpp
    ${CMAKE_SOURCE_DIR}/src/codegen.cpp
    ${CMAKE_SOURCE_DIR}/src/exported.cpp
    ${CMAKE_SOURCE_DIR}/src/driver.cpp
)
target_precompile_headers(demo PUBLIC ${CMAKE_SOURCE_DIR}/src/prec.h)
target_include_directories(demo PRIVATE
    ${CMAKE_SOURCE_DIR}/src/
    ${CMAKE_SOURCE_DIR}/deps/gflags/include/
    ${CMAKE_SOURCE_DIR}/deps/glog/include/
    ${CMAKE_SOURCE_DIR}/deps/fmt/include/
    ${CMAKE_SOURCE_DIR}/deps/boost/include/boost-1_76/
    ${CMAKE_SOURCE_DIR}/deps/llvm/include/
)
execute_process (
    COMMAND ${CMAKE_SOURCE_DIR}/deps/llvm/bin/llvm-config.exe --libs core orcjit native
    OUTPUT_STRIP_TRAILING_WHITESPACE
    OUTPUT_VARIABLE LLVM_LIBRARIES
)
separate_arguments(LLVM_LIBRARIES)
# message(STATUS "llvm libraries => ${LLVM_LIBRARIES}")
target_link_libraries(demo antlr4_runtime shlwapi.lib
    ${CMAKE_SOURCE_DIR}/deps/gflags/lib/gflags_static.lib
    ${CMAKE_SOURCE_DIR}/deps/glog/lib/glog.lib
    ${CMAKE_SOURCE_DIR}/deps/fmt/lib/fmt.lib
    ${LLVM_LIBRARIES}
)
set_target_properties(demo PROPERTIES LINK_FLAGS "/manifest:no")
