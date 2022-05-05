add_library(deps_gflags INTERFACE)
target_include_directories(deps_gflags INTERFACE ${DEPS_PREFIX}/gflags/include)
target_link_libraries(deps_gflags INTERFACE ${DEPS_PREFIX}/gflags/lib/libgflags.a pthread)

add_library(deps_glog INTERFACE)
target_include_directories(deps_glog INTERFACE ${DEPS_PREFIX}/glog/include)
target_link_libraries(deps_glog INTERFACE ${DEPS_PREFIX}/glog/lib/libglog.a)

add_library(deps_fmt INTERFACE)
target_include_directories(deps_fmt INTERFACE ${DEPS_PREFIX}/fmt/include)
target_link_libraries(deps_fmt INTERFACE ${DEPS_PREFIX}/fmt/lib/libfmt.a)

add_library(deps_expected-lite INTERFACE)
target_include_directories(deps_expected-lite INTERFACE ${DEPS_PREFIX}/expected-lite/include)

add_library(deps_python3 INTERFACE)
target_include_directories(deps_python3 INTERFACE ${DEPS_PREFIX}/python3/include/python3.8)
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

add_library(deps_pybind11 INTERFACE)
target_include_directories(deps_pybind11 INTERFACE ${DEPS_PREFIX}/pybind11/include)
target_link_libraries(deps_pybind11 INTERFACE deps_python3)

add_library(deps_glfw INTERFACE)
target_include_directories(deps_glfw INTERFACE ${DEPS_PREFIX}/glfw/include)
target_link_libraries(deps_glfw INTERFACE ${DEPS_PREFIX}/glfw/lib/libglfw3.a dl X11)

add_library(deps_mujoco INTERFACE)
target_include_directories(deps_mujoco INTERFACE ${DEPS_PREFIX}/mujoco/include)
target_link_libraries(deps_mujoco INTERFACE ${DEPS_PREFIX}/mujoco/lib/libmujoco.so deps_glfw)

add_library(deps_torch INTERFACE)
target_include_directories(deps_torch INTERFACE
    ${DEPS_PREFIX}/torch/include/
    ${DEPS_PREFIX}/torch/include/torch/csrc/api/include/
)
target_link_libraries(deps_torch INTERFACE
    ${DEPS_PREFIX}/torch/lib/libc10.so
    ${DEPS_PREFIX}/torch/lib/libgomp-75eea7e8.so.1
    ${DEPS_PREFIX}/torch/lib/libtorch_cpu.so
)

# add_library(deps_imgui INTERFACE)
# target_link_libraries(deps_imgui INTERFACE deps_header_only
#     opengl32.lib
#     ${DEPS_PREFIX}/lib/imgui.lib
# )

# add_library(deps_implot INTERFACE)
# target_link_libraries(deps_implot INTERFACE deps_header_only
#     ${DEPS_PREFIX}/lib/implot.lib
# )
