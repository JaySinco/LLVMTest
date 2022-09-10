get_filename_component(CURRENT_DIR_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
set(imgui_BINGINGS_DIR ${imgui_ROOT}/res/bindings)
set(TESTING ${CURRENT_DIR_NAME}-testing)

add_executable(${TESTING}
    env/env.cpp
    policy/policy.cpp
    policy/pg.cpp
    testing.cpp
    ${imgui_BINGINGS_DIR}/imgui_impl_glfw.cpp
    ${imgui_BINGINGS_DIR}/imgui_impl_opengl2.cpp
)

target_include_directories(${TESTING} PRIVATE ${imgui_BINGINGS_DIR})
target_precompile_headers(${TESTING} PRIVATE prec.h)

target_link_libraries(${TESTING} utils
    implot::implot
    glfw::glfw
    mujoco::mujoco
    Torch::torch
)

add_custom_command(TARGET ${TESTING} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${mujoco_ROOT}/$<IF:$<BOOL:${UNIX}>,lib/libmujoco.so.${mujoco_VERSION_STRING},bin/mujoco.dll>
    ${torch_ROOT}/bin/$<IF:$<BOOL:${UNIX}>,,libiomp5md.dll>
    $<TARGET_FILE_DIR:${TESTING}>
)
