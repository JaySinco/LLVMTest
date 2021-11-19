file(GLOB_RECURSE CALC_GENCPP_SRC .antlr/*.cpp)

add_executable(parser-testing
    ${CALC_GENCPP_SRC}
    testing.cpp
)

target_compile_definitions(parser-testing PRIVATE
    _CRT_SECURE_NO_WARNINGS
    _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
)

target_include_directories(parser-testing PRIVATE
    ${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include/antlr4-runtime
)

target_link_libraries(parser-testing utils
    ${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib/antlr4-runtime.lib
)
