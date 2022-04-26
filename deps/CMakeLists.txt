add_library(deps_header_only INTERFACE)
target_include_directories(deps_header_only INTERFACE
   ${PROTO_VCPKG_PREFIX}/include/
)

add_library(deps_fmt INTERFACE)
target_link_libraries(deps_fmt INTERFACE deps_header_only
    ${PROTO_VCPKG_PREFIX}/lib/fmt.lib
)

add_library(deps_gflags INTERFACE)
target_link_libraries(deps_gflags INTERFACE deps_header_only
    ${PROTO_VCPKG_PREFIX}/lib/gflags.lib
)

add_library(deps_glog INTERFACE)
target_link_libraries(deps_glog INTERFACE deps_header_only
    ${PROTO_VCPKG_PREFIX}/lib/glog.lib
)

add_library(deps_torch INTERFACE)
target_include_directories(deps_torch INTERFACE
    ${PROTO_VCPKG_PREFIX}/etc/libtorch/include/
    ${PROTO_VCPKG_PREFIX}/etc/libtorch/include/torch/csrc/api/include/
)
target_link_libraries(deps_torch INTERFACE
    "-INCLUDE:?warp_size@cuda@at@@YAHXZ"
    "-INCLUDE:?searchsorted_cuda@native@at@@YA?AVTensor@2@AEBV32@0_N1@Z"
    ${PROTO_VCPKG_PREFIX}/etc/libtorch/lib/c10.lib
    ${PROTO_VCPKG_PREFIX}/etc/libtorch/lib/torch_cpu.lib
    ${PROTO_VCPKG_PREFIX}/etc/libtorch/lib/torch_cuda_cpp.lib
    ${PROTO_VCPKG_PREFIX}/etc/libtorch/lib/torch_cuda_cu.lib
)

add_library(deps_mujoco INTERFACE)
target_link_libraries(deps_mujoco INTERFACE deps_header_only
    ${PROTO_VCPKG_PREFIX}/lib/mujoco210.lib
    ${PROTO_VCPKG_PREFIX}/lib/glfw3dll.lib
)

add_library(deps_imgui INTERFACE)
target_link_libraries(deps_imgui INTERFACE deps_header_only
    opengl32.lib
    ${PROTO_VCPKG_PREFIX}/lib/imgui.lib
)

add_library(deps_implot INTERFACE)
target_link_libraries(deps_implot INTERFACE deps_header_only
    ${PROTO_VCPKG_PREFIX}/lib/implot.lib
)

add_library(deps_python3 INTERFACE)
target_include_directories(deps_python3 INTERFACE
    ${PROTO_VCPKG_PREFIX}/include/python3
)
target_link_libraries(deps_python3 INTERFACE
    ${PROTO_VCPKG_PREFIX}/lib/python38.lib
)
