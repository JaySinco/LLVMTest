get_filename_component(CURRENT_DIR_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
set(TESTING ${CURRENT_DIR_NAME}-testing)

if(NOT EXISTS ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/libtorch.so)
    file(COPY ${DEPS_PREFIX}/torch/lib/
        DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
        FILES_MATCHING
        PATTERN "*.so"
        PATTERN "*.so.1"
    )
endif()

# add_executable(${TESTING}
#     env/env.cpp
#     policy/policy.cpp
#     policy/pg.cpp
#     testing.cpp
# )
# target_compile_options(${TESTING} PRIVATE -Wno-nonportable-include-path)
# target_precompile_headers(${TESTING} PRIVATE prec.h)
# target_link_libraries(${TESTING} utils deps_mujoco deps_torch deps_imgui deps_implot)

add_executable(simulate testbed/uitools.cpp testbed/simulate.cpp)
target_link_libraries(simulate utils deps_mujoco)
add_custom_command(TARGET simulate POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${DEPS_PREFIX}/mujoco/lib/libmujoco.so.2.1.5
    $<TARGET_FILE_DIR:simulate>
)

add_executable(linear-regression testbed/linear-regression.cpp)
# target_precompile_headers(linear-regression REUSE_FROM ${TESTING})
target_link_libraries(linear-regression utils deps_torch)

add_executable(fashion-mnist testbed/fashion-mnist.cpp)
# target_precompile_headers(fashion-mnist REUSE_FROM ${TESTING})
set_target_properties(fashion-mnist PROPERTIES ENABLE_EXPORTS 1)
target_link_libraries(fashion-mnist utils deps_python3 deps_torch)
