add_executable(simulate
    uitools.c
    simulate.cpp
)

target_compile_options(simulate PRIVATE -Wno-deprecated-declarations)

target_include_directories(simulate PRIVATE
    ${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include/
)

target_link_libraries(simulate
    ${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib/mujoco210.lib
    ${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib/glfw3dll.lib
)

set_target_properties(simulate PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/mujoco"
)
