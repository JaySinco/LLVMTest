find_package(LLVM CONFIG REQUIRED)

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
    ${LLVM_INCLUDE_DIRS}
    ${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include/antlr4-runtime
)

llvm_map_components_to_libnames(llvm_libs core orcjit native)

target_link_libraries(calc
    utils
    ${llvm_libs}
    ${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}$<$<CONFIG:Debug>:/debug>/lib/antlr4-runtime.lib
)

set_target_properties(calc PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/calc"
)
