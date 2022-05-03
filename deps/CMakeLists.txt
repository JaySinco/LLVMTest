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

# target_link_libraries(deps_gflags INTERFACE
#     ${DEPS_PREFIX}/lib/gflags.lib
# )

# add_library(deps_glog INTERFACE)
# target_link_libraries(deps_glog INTERFACE
#     ${DEPS_PREFIX}/lib/glog.lib
# )

# add_library(deps_torch INTERFACE)
# target_include_directories(deps_torch INTERFACE
#     ${DEPS_PREFIX}/etc/libtorch/include/
#     ${DEPS_PREFIX}/etc/libtorch/include/torch/csrc/api/include/
# )
# target_link_libraries(deps_torch INTERFACE
#     "-INCLUDE:?warp_size@cuda@at@@YAHXZ"
#     "-INCLUDE:?searchsorted_cuda@native@at@@YA?AVTensor@2@AEBV32@0_N1@Z"
#     ${DEPS_PREFIX}/etc/libtorch/lib/c10.lib
#     ${DEPS_PREFIX}/etc/libtorch/lib/torch_cpu.lib
#     ${DEPS_PREFIX}/etc/libtorch/lib/torch_cuda_cpp.lib
#     ${DEPS_PREFIX}/etc/libtorch/lib/torch_cuda_cu.lib
# )

# add_library(deps_mujoco INTERFACE)
# target_link_libraries(deps_mujoco INTERFACE deps_header_only
#     ${DEPS_PREFIX}/lib/mujoco210.lib
#     ${DEPS_PREFIX}/lib/glfw3dll.lib
# )

# add_library(deps_imgui INTERFACE)
# target_link_libraries(deps_imgui INTERFACE deps_header_only
#     opengl32.lib
#     ${DEPS_PREFIX}/lib/imgui.lib
# )

# add_library(deps_implot INTERFACE)
# target_link_libraries(deps_implot INTERFACE deps_header_only
#     ${DEPS_PREFIX}/lib/implot.lib
# )

# add_library(deps_python3 INTERFACE)
# target_include_directories(deps_python3 INTERFACE
#     ${DEPS_PREFIX}/include/python3
# )
# target_link_libraries(deps_python3 INTERFACE
#     ${DEPS_PREFIX}/lib/python38.lib
# )
