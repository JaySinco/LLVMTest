get_filename_component(CURRENT_DIR_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
set(TESTING_EXE ${CURRENT_DIR_NAME}-testing)

if(NOT EXISTS ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/torch.dll)
    file(COPY ${PROTO_VCPKG_PREFIX}/etc/libtorch/lib/
        DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
        FILES_MATCHING PATTERN "*.dll"
    )
endif()

add_executable(${TESTING_EXE}
    env/env.cpp
    policy/policy.cpp
    policy/pg.cpp
    testing.cpp
)
target_compile_options(${TESTING_EXE} PRIVATE -Wno-nonportable-include-path)
target_precompile_headers(${TESTING_EXE} PRIVATE prec.h)
target_link_libraries(${TESTING_EXE} utils deps_mujoco deps_torch deps_imgui deps_implot)

add_executable(simulate testbed/uitools.cpp testbed/simulate.cpp)
target_compile_options(simulate PRIVATE -Wno-deprecated-declarations -Wno-nonportable-include-path)
target_link_libraries(simulate utils deps_mujoco)

add_executable(linear-regression testbed/linear-regression.cpp)
target_precompile_headers(linear-regression REUSE_FROM ${TESTING_EXE})
target_link_libraries(linear-regression utils deps_torch)

add_executable(fashion-mnist testbed/fashion-mnist.cpp)
target_precompile_headers(fashion-mnist REUSE_FROM ${TESTING_EXE})
target_link_libraries(fashion-mnist utils deps_python3 deps_torch)
