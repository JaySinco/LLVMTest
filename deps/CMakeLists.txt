add_library(deps_gflags INTERFACE)
target_include_directories(deps_gflags INTERFACE ${DEPS_PREFIX}/gflags/include)
if(UNIX)
    target_link_libraries(deps_gflags INTERFACE ${DEPS_PREFIX}/gflags/lib/libgflags.a pthread)
elseif(WIN32)
    target_link_libraries(deps_gflags INTERFACE ${DEPS_PREFIX}/gflags/lib/gflags_static.lib)
endif()

add_library(deps_glog INTERFACE)
target_include_directories(deps_glog INTERFACE ${DEPS_PREFIX}/glog/include)
if(UNIX)
    target_link_libraries(deps_glog INTERFACE ${DEPS_PREFIX}/glog/lib/libglog.a)
elseif(WIN32)
    target_compile_definitions(deps_glog INTERFACE GLOG_NO_ABBREVIATED_SEVERITIES)
    target_link_libraries(deps_glog INTERFACE ${DEPS_PREFIX}/glog/lib/glog.lib shlwapi.lib)
endif()

add_library(deps_fmt INTERFACE)
target_include_directories(deps_fmt INTERFACE ${DEPS_PREFIX}/fmt/include)
if(UNIX)
    target_link_libraries(deps_fmt INTERFACE ${DEPS_PREFIX}/fmt/lib/libfmt.a)
elseif(WIN32)
    target_link_libraries(deps_fmt INTERFACE ${DEPS_PREFIX}/fmt/lib/fmt.lib)
endif()

add_library(deps_expected-lite INTERFACE)
target_include_directories(deps_expected-lite INTERFACE ${DEPS_PREFIX}/expected-lite/include)

add_library(deps_python3 INTERFACE)
target_include_directories(deps_python3 INTERFACE ${DEPS_PREFIX}/python3/include/python3.8)
if(UNIX)
    target_link_libraries(deps_python3 INTERFACE ${DEPS_PREFIX}/python3/lib/libpython3.8.a dl util)
    string(CONCAT DEPS_PYTHON_SYS_PATH
        "${DEPS_PREFIX}/python3/lib/python38.zip:"
        "${DEPS_PREFIX}/python3/lib/python3.8:"
        "${DEPS_PREFIX}/python3/lib/python3.8/lib-dynload:"
        "${DEPS_PREFIX}/python3/lib/python3.8/site-packages"
    )
    target_compile_definitions(deps_python3 INTERFACE
        DEPS_PYTHON_SYS_PATH="\"\"${DEPS_PYTHON_SYS_PATH}\"\""
    )
elseif(WIN32)
    target_link_libraries(deps_python3 INTERFACE ${DEPS_PREFIX}/python3/lib/python38.lib)
endif()

add_library(deps_pybind11 INTERFACE)
target_include_directories(deps_pybind11 INTERFACE ${DEPS_PREFIX}/pybind11/include)
target_link_libraries(deps_pybind11 INTERFACE deps_python3)

add_library(deps_glfw INTERFACE)
target_include_directories(deps_glfw INTERFACE ${DEPS_PREFIX}/glfw/include)
if(UNIX)
    target_link_libraries(deps_glfw INTERFACE ${DEPS_PREFIX}/glfw/lib/libglfw3.a dl X11 GL)
elseif(WIN32)
    target_link_libraries(deps_glfw INTERFACE ${DEPS_PREFIX}/glfw/lib/glfw3.lib opengl32.lib)
endif()

add_library(deps_mujoco INTERFACE)
target_include_directories(deps_mujoco INTERFACE ${DEPS_PREFIX}/mujoco/include)
if(UNIX)
    target_link_libraries(deps_mujoco INTERFACE ${DEPS_PREFIX}/mujoco/lib/libmujoco.so deps_glfw)
elseif(WIN32)
    target_link_libraries(deps_mujoco INTERFACE ${DEPS_PREFIX}/mujoco/lib/mujoco.lib deps_glfw)
endif()

add_library(deps_torch INTERFACE)
target_include_directories(deps_torch INTERFACE
    ${DEPS_PREFIX}/torch/include/
    ${DEPS_PREFIX}/torch/include/torch/csrc/api/include/
)
if(UNIX)
    target_link_libraries(deps_torch INTERFACE
        ${DEPS_PREFIX}/torch/lib/libc10.so
        ${DEPS_PREFIX}/torch/lib/libgomp-75eea7e8.so.1
        ${DEPS_PREFIX}/torch/lib/libtorch_cpu.so
    )
elseif(WIN32)
    target_compile_options(deps_torch INTERFACE /W0)
    target_link_libraries(deps_torch INTERFACE
        "-INCLUDE:?warp_size@cuda@at@@YAHXZ"
        "-INCLUDE:?searchsorted_cuda@native@at@@YA?AVTensor@2@AEBV32@0_N1@Z"
        ${DEPS_PREFIX}/torch/lib/c10.lib
        ${DEPS_PREFIX}/torch/lib/torch_cpu.lib
        ${DEPS_PREFIX}/torch/lib/torch_cuda_cpp.lib
        ${DEPS_PREFIX}/torch/lib/torch_cuda_cu.lib
    )
endif()

add_library(deps_imgui STATIC
    ${DEPS_PREFIX}/imgui/lib/imgui.cpp
    ${DEPS_PREFIX}/imgui/lib/imgui_demo.cpp
    ${DEPS_PREFIX}/imgui/lib/imgui_draw.cpp
    ${DEPS_PREFIX}/imgui/lib/imgui_tables.cpp
    ${DEPS_PREFIX}/imgui/lib/imgui_widgets.cpp
    ${DEPS_PREFIX}/imgui/lib/imgui_impl_glfw.cpp
    ${DEPS_PREFIX}/imgui/lib/imgui_impl_opengl2.cpp
)
target_include_directories(deps_imgui PUBLIC ${DEPS_PREFIX}/imgui/include)
target_link_libraries(deps_imgui PUBLIC deps_glfw)

add_library(deps_implot STATIC
    ${DEPS_PREFIX}/implot/lib/implot.cpp
    ${DEPS_PREFIX}/implot/lib/implot_items.cpp
    ${DEPS_PREFIX}/implot/lib/implot_demo.cpp
)
target_include_directories(deps_implot PUBLIC ${DEPS_PREFIX}/implot/include)
target_link_libraries(deps_implot PUBLIC deps_imgui)

add_library(deps_boost INTERFACE)
target_compile_definitions(deps_boost INTERFACE BOOST_ALL_NO_LIB)
if(UNIX)
    target_include_directories(deps_boost INTERFACE ${DEPS_PREFIX}/boost/include)
    target_link_libraries(deps_boost INTERFACE
        ${DEPS_PREFIX}/boost/lib/libboost_filesystem.a
        ${DEPS_PREFIX}/boost/lib/libboost_coroutine.a
        ${DEPS_PREFIX}/boost/lib/libboost_context.a
        ${DEPS_PREFIX}/boost/lib/libboost_timer.a
        ${DEPS_PREFIX}/boost/lib/libboost_chrono.a
        ${DEPS_PREFIX}/boost/lib/libboost_thread.a
    )
elseif(WIN32)
    target_include_directories(deps_boost INTERFACE ${DEPS_PREFIX}/boost/include/boost-1_79)
    target_link_libraries(deps_boost INTERFACE
        ${DEPS_PREFIX}/boost/lib/libboost_filesystem-vc142-mt-x64-1_79.lib
        ${DEPS_PREFIX}/boost/lib/libboost_coroutine-vc142-mt-x64-1_79.lib
        ${DEPS_PREFIX}/boost/lib/libboost_context-vc142-mt-x64-1_79.lib
        ${DEPS_PREFIX}/boost/lib/libboost_timer-vc142-mt-x64-1_79.lib
        ${DEPS_PREFIX}/boost/lib/libboost_chrono-vc142-mt-x64-1_79.lib
        ${DEPS_PREFIX}/boost/lib/libboost_thread-vc142-mt-x64-1_79.lib
    )
endif()
