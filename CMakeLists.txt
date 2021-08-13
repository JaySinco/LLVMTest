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
target_compile_definitions(antlr4_runtime PUBLIC ANTLR4CPP_STATIC)

file(GLOB_RECURSE GEN_CPP_SRC ${CMAKE_SOURCE_DIR}/src/gen-cpp/*.cpp)
add_executable(demo
    ${GEN_CPP_SRC}
    ${CMAKE_SOURCE_DIR}/src/utils.cpp
    ${CMAKE_SOURCE_DIR}/src/driver.cpp
)
target_include_directories(demo PRIVATE
    ${CMAKE_SOURCE_DIR}/src/
    ${CMAKE_SOURCE_DIR}/deps/gflags/include/
    ${CMAKE_SOURCE_DIR}/deps/glog/include/
    ${CMAKE_SOURCE_DIR}/deps/fmt/include/
    ${CMAKE_SOURCE_DIR}/deps/boost/include/boost-1_76/
    ${CMAKE_SOURCE_DIR}/deps/llvm/include/
)
target_link_libraries(demo antlr4_runtime shlwapi.lib 
    ${CMAKE_SOURCE_DIR}/deps/gflags/lib/gflags_static.lib
    ${CMAKE_SOURCE_DIR}/deps/glog/lib/glog.lib
    ${CMAKE_SOURCE_DIR}/deps/fmt/lib/fmt.lib
    ${CMAKE_SOURCE_DIR}/deps/llvm/lib/LLVMCore.lib
    ${CMAKE_SOURCE_DIR}/deps/llvm/lib/LLVMRemarks.lib
    ${CMAKE_SOURCE_DIR}/deps/llvm/lib/LLVMBitstreamReader.lib
    ${CMAKE_SOURCE_DIR}/deps/llvm/lib/LLVMBinaryFormat.lib
    ${CMAKE_SOURCE_DIR}/deps/llvm/lib/LLVMSupport.lib
    ${CMAKE_SOURCE_DIR}/deps/llvm/lib/LLVMDemangle.lib
)
set_target_properties(demo PROPERTIES LINK_FLAGS "/manifest:no")
