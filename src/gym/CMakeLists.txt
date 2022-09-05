get_filename_component(CURRENT_DIR_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
set(TESTING ${CURRENT_DIR_NAME}-testing)

add_executable(${TESTING}
    env/env.cpp
    policy/policy.cpp
    policy/pg.cpp
    testing.cpp
    ${IMGUI_BINGINGS_DIR}/imgui_impl_glfw.cpp
    ${IMGUI_BINGINGS_DIR}/imgui_impl_opengl2.cpp
)

target_include_directories(${TESTING} PRIVATE ${IMGUI_BINGINGS_DIR})
target_precompile_headers(${TESTING} PRIVATE prec.h)

target_link_libraries(${TESTING} utils
    implot::implot
    glfw::glfw
    deps_mujoco deps_torch
)

add_custom_command(TARGET ${TESTING} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${DEPS_PREFIX}/mujoco/lib/$<IF:$<BOOL:${UNIX}>,libmujoco.so.2.1.5,mujoco.dll>
    ${DEPS_PREFIX}/torch/lib/$<IF:$<BOOL:${UNIX}>,,libiomp5md.dll>
    $<TARGET_FILE_DIR:${TESTING}>
)
