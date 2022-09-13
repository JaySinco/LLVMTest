get_filename_component(CURRENT_DIR_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
set(imgui_BINGINGS_DIR ${imgui_ROOT}/res/bindings)
set(MAIN ${CURRENT_DIR_NAME})

add_executable(${MAIN}
    env.cpp
    policy.cpp
    pg.cpp
    main.cpp
    ${imgui_BINGINGS_DIR}/imgui_impl_glfw.cpp
    ${imgui_BINGINGS_DIR}/imgui_impl_opengl2.cpp
)

target_include_directories(${MAIN} PRIVATE ${imgui_BINGINGS_DIR})
target_precompile_headers(${MAIN} PRIVATE prec.h)

target_link_libraries(${MAIN} utils
    implot::implot
    glfw::glfw
    mujoco::mujoco
    Torch::torch
    $<IF:$<BOOL:${UNIX}>,,-INCLUDE:?warp_size@cuda@at@@YAHXZ>
    $<IF:$<BOOL:${UNIX}>,,-INCLUDE:?searchsorted_cuda@native@at@@YA?AVTensor@2@AEBV32@0_N1@Z>
)

add_custom_command(TARGET ${MAIN} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${mujoco_ROOT}/$<IF:$<BOOL:${UNIX}>,lib/libmujoco.so.${mujoco_VERSION_STRING},bin/mujoco.dll>
    ${torch_ROOT}/bin/$<IF:$<BOOL:${UNIX}>,,libiomp5md.dll>
    $<TARGET_FILE_DIR:${MAIN}>
)
