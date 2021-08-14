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
    ${CMAKE_SOURCE_DIR}/src/codegen.cpp
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
target_link_libraries(demo antlr4_runtime shlwapi.lib psapi.lib shell32.lib
    ole32.lib uuid.lib advapi32.lib
    ${CMAKE_SOURCE_DIR}/deps/gflags/lib/gflags_static.lib
    ${CMAKE_SOURCE_DIR}/deps/glog/lib/glog.lib
    ${CMAKE_SOURCE_DIR}/deps/fmt/lib/fmt.lib
    ${CMAKE_SOURCE_DIR}/deps/llvm/lib/LLVMX86Disassembler.lib
    ${CMAKE_SOURCE_DIR}/deps/llvm/lib/LLVMX86AsmParser.lib 
    ${CMAKE_SOURCE_DIR}/deps/llvm/lib/LLVMX86CodeGen.lib 
    ${CMAKE_SOURCE_DIR}/deps/llvm/lib/LLVMCFGuard.lib 
    ${CMAKE_SOURCE_DIR}/deps/llvm/lib/LLVMGlobalISel.lib 
    ${CMAKE_SOURCE_DIR}/deps/llvm/lib/LLVMSelectionDAG.lib 
    ${CMAKE_SOURCE_DIR}/deps/llvm/lib/LLVMAsmPrinter.lib 
    ${CMAKE_SOURCE_DIR}/deps/llvm/lib/LLVMDebugInfoDWARF.lib 
    ${CMAKE_SOURCE_DIR}/deps/llvm/lib/LLVMCodeGen.lib 
    ${CMAKE_SOURCE_DIR}/deps/llvm/lib/LLVMScalarOpts.lib
    ${CMAKE_SOURCE_DIR}/deps/llvm/lib/LLVMInstCombine.lib 
    ${CMAKE_SOURCE_DIR}/deps/llvm/lib/LLVMAggressiveInstCombine.lib
    ${CMAKE_SOURCE_DIR}/deps/llvm/lib/LLVMTransformUtils.lib 
    ${CMAKE_SOURCE_DIR}/deps/llvm/lib/LLVMBitWriter.lib 
    ${CMAKE_SOURCE_DIR}/deps/llvm/lib/LLVMX86Desc.lib 
    ${CMAKE_SOURCE_DIR}/deps/llvm/lib/LLVMMCDisassembler.lib
    ${CMAKE_SOURCE_DIR}/deps/llvm/lib/LLVMX86Info.lib 
    ${CMAKE_SOURCE_DIR}/deps/llvm/lib/LLVMMCJIT.lib 
    ${CMAKE_SOURCE_DIR}/deps/llvm/lib/LLVMExecutionEngine.lib 
    ${CMAKE_SOURCE_DIR}/deps/llvm/lib/LLVMTarget.lib 
    ${CMAKE_SOURCE_DIR}/deps/llvm/lib/LLVMAnalysis.lib 
    ${CMAKE_SOURCE_DIR}/deps/llvm/lib/LLVMProfileData.lib 
    ${CMAKE_SOURCE_DIR}/deps/llvm/lib/LLVMRuntimeDyld.lib 
    ${CMAKE_SOURCE_DIR}/deps/llvm/lib/LLVMObject.lib 
    ${CMAKE_SOURCE_DIR}/deps/llvm/lib/LLVMTextAPI.lib 
    ${CMAKE_SOURCE_DIR}/deps/llvm/lib/LLVMMCParser.lib 
    ${CMAKE_SOURCE_DIR}/deps/llvm/lib/LLVMBitReader.lib 
    ${CMAKE_SOURCE_DIR}/deps/llvm/lib/LLVMMC.lib 
    ${CMAKE_SOURCE_DIR}/deps/llvm/lib/LLVMDebugInfoCodeView.lib 
    ${CMAKE_SOURCE_DIR}/deps/llvm/lib/LLVMDebugInfoMSF.lib 
    ${CMAKE_SOURCE_DIR}/deps/llvm/lib/LLVMCore.lib 
    ${CMAKE_SOURCE_DIR}/deps/llvm/lib/LLVMRemarks.lib 
    ${CMAKE_SOURCE_DIR}/deps/llvm/lib/LLVMBitstreamReader.lib
    ${CMAKE_SOURCE_DIR}/deps/llvm/lib/LLVMBinaryFormat.lib 
    ${CMAKE_SOURCE_DIR}/deps/llvm/lib/LLVMSupport.lib 
    ${CMAKE_SOURCE_DIR}/deps/llvm/lib/LLVMDemangle.lib
)
set_target_properties(demo PROPERTIES LINK_FLAGS "/manifest:no")
