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

execute_process (
    COMMAND cmake -E remove_directory gen-cpp
    COMMAND java -jar ${CMAKE_SOURCE_DIR}/deps/antlr4/bin/antlr-4.9.2-complete.jar
        -encoding utf8 -Dlanguage=Cpp -Werror -no-listener -no-visitor -o gen-cpp T*.g4
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND_ERROR_IS_FATAL ANY
)

file(GLOB_RECURSE CALC_GENCPP_SRC gen-cpp/*.cpp)
add_executable(calc
    ${CALC_GENCPP_SRC}
    jit-compiler.cpp
    codegen.cpp
    exported.cpp
    driver.cpp
)

target_precompile_headers(calc PUBLIC prec.h)

target_include_directories(calc PRIVATE
    ${CMAKE_SOURCE_DIR}/deps/llvm/include/
)

execute_process (
    COMMAND ${CMAKE_SOURCE_DIR}/deps/llvm/bin/llvm-config.exe --libs core orcjit native
    OUTPUT_STRIP_TRAILING_WHITESPACE
    OUTPUT_VARIABLE LLVM_LIBRARIES
)
separate_arguments(LLVM_LIBRARIES)

target_link_libraries(calc utils antlr4_runtime ${LLVM_LIBRARIES})
set_target_properties(calc PROPERTIES LINK_FLAGS "/manifest:no")
