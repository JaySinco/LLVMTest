get_filename_component(CURRENT_DIR_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
set(IMGUI_BINGINGS_DIR ${IMGUI_ROOT}/res/bindings)
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
    mujoco::mujoco
    Torch::torch
    $<IF:$<BOOL:${UNIX}>,,-INCLUDE:?warp_size@cuda@at@@YAHXZ>
    $<IF:$<BOOL:${UNIX}>,,-INCLUDE:?searchsorted_cuda@native@at@@YA?AVTensor@2@AEBV32@0_N1@Z>
)

add_custom_command(TARGET ${TESTING} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${MUJOCO_ROOT}/$<IF:$<BOOL:${UNIX}>,lib/libmujoco.so.${mujoco_VERSION_STRING},bin/mujoco.dll>
    ${TORCH_ROOT}/bin/$<IF:$<BOOL:${UNIX}>,,libiomp5md.dll>
    $<TARGET_FILE_DIR:${TESTING}>
)
