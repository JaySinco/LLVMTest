get_filename_component(CURRENT_DIR_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
set(TESTING ${CURRENT_DIR_NAME}-testing)

add_executable(${TESTING}
    env/env.cpp
    policy/policy.cpp
    policy/pg.cpp
    testing.cpp
)
target_precompile_headers(${TESTING} PRIVATE prec.h)
target_link_libraries(${TESTING} utils deps_mujoco deps_torch deps_imgui deps_implot)
add_custom_command(TARGET ${TESTING} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${DEPS_PREFIX}/mujoco/lib/$<IF:$<BOOL:${UNIX}>,libmujoco.so.2.1.5,mujoco.dll>
    $<TARGET_FILE_DIR:${TESTING}>
)

add_executable(simulate testbed/uitools.cpp testbed/simulate.cpp)
target_compile_options(simulate PRIVATE $<$<CXX_COMPILER_ID:Clang>:-Wno-deprecated-declarations>)
target_link_libraries(simulate utils deps_mujoco)

add_executable(linear-regression testbed/linear-regression.cpp)
target_precompile_headers(linear-regression REUSE_FROM ${TESTING})
target_link_libraries(linear-regression utils deps_torch)

add_executable(fashion-mnist testbed/fashion-mnist.cpp)
target_precompile_headers(fashion-mnist REUSE_FROM ${TESTING})
if(UNIX)
    set_target_properties(fashion-mnist PROPERTIES ENABLE_EXPORTS 1)
endif()
target_link_libraries(fashion-mnist utils deps_python3 deps_torch)
