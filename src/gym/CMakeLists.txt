get_filename_component(CURRENT_DIR_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)

set(TORCH_ROOT_DIR ${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/etc/libtorch/)

add_library(libtorch INTERFACE)

target_include_directories(libtorch INTERFACE
    ${TORCH_ROOT_DIR}/include/
    ${TORCH_ROOT_DIR}/include/torch/csrc/api/include/
)

target_link_libraries(libtorch INTERFACE
    "-INCLUDE:?warp_size@cuda@at@@YAHXZ"
    "-INCLUDE:?searchsorted_cuda@native@at@@YA?AVTensor@2@AEBV32@0_N1@Z"
    ${TORCH_ROOT_DIR}/lib/c10.lib
    ${TORCH_ROOT_DIR}/lib/torch_cpu.lib
    ${TORCH_ROOT_DIR}/lib/torch_cuda_cpp.lib
    ${TORCH_ROOT_DIR}/lib/torch_cuda_cu.lib
)

if(NOT EXISTS ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/torch.dll)
    file(COPY ${TORCH_ROOT_DIR}/lib/
        DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
        FILES_MATCHING PATTERN "*.dll"
    )
endif()

add_library(mujoco INTERFACE)

target_link_libraries(mujoco INTERFACE
    ${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib/mujoco210.lib
    ${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib/glfw3dll.lib
)

add_executable(${CURRENT_DIR_NAME}-testing
    env/env.cpp
    policy/policy.cpp
    policy/pg.cpp
    testing.cpp
)

target_precompile_headers(${CURRENT_DIR_NAME}-testing PRIVATE prec.h)

target_link_libraries(${CURRENT_DIR_NAME}-testing utils mujoco libtorch opengl32.lib
    ${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib/imgui.lib
    ${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib/implot.lib
)

add_executable(simulate testbed/uitools.cpp testbed/simulate.cpp)
target_compile_options(simulate PRIVATE -Wno-deprecated-declarations)
target_link_libraries(simulate utils mujoco)

add_executable(linear-regression testbed/linear-regression.cpp)
target_precompile_headers(linear-regression REUSE_FROM ${CURRENT_DIR_NAME}-testing)
target_link_libraries(linear-regression utils libtorch)

add_executable(fashion-mnist testbed/fashion-mnist.cpp)
target_precompile_headers(fashion-mnist REUSE_FROM ${CURRENT_DIR_NAME}-testing)
target_link_libraries(fashion-mnist utils py3 libtorch)
