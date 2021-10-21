add_library(mujoco INTERFACE)

target_include_directories(mujoco INTERFACE
    ${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include/
)

target_link_libraries(mujoco INTERFACE
    ${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib/mujoco210.lib
    ${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib/glfw3dll.lib
)

add_executable(simulate uitools.cpp simulate.cpp)

target_compile_options(simulate PRIVATE -Wno-deprecated-declarations)

target_link_libraries(simulate mujoco)

set_target_properties(simulate PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/mujoco"
)

add_executable(testing-mujoco testing.cpp)

target_link_libraries(testing-mujoco utils mujoco)

set_target_properties(testing-mujoco PROPERTIES
    OUTPUT_NAME "testing"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/mujoco"
)
