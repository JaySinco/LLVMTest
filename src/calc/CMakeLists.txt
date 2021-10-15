file(GLOB_RECURSE CALC_GENCPP_SRC .antlr/*.cpp)

add_executable(calc
    ${CALC_GENCPP_SRC}
    jit-compiler.cpp
    codegen.cpp
    exported.cpp
    main.cpp
)

target_compile_definitions(calc PRIVATE
    _CRT_SECURE_NO_WARNINGS
    _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
)

target_precompile_headers(calc PRIVATE prec.h)

target_include_directories(calc PRIVATE
    ${CMAKE_SOURCE_DIR}/deps/llvm/include/
    ${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include/antlr4-runtime
)

execute_process(
    COMMAND ${CMAKE_SOURCE_DIR}/deps/llvm/bin/llvm-config.exe --libs core orcjit native
    OUTPUT_STRIP_TRAILING_WHITESPACE
    OUTPUT_VARIABLE llvm_libs
    COMMAND_ERROR_IS_FATAL ANY
)

separate_arguments(llvm_libs)

target_link_libraries(calc
    utils
    ${llvm_libs}
    ${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib/antlr4-runtime.lib
)

set_target_properties(calc PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/calc"
)
