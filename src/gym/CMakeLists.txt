get_filename_component(CURRENT_DIR_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)

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
target_link_libraries(simulate utils mujoco)

add_executable(${CURRENT_DIR_NAME}-testing
    env/env.cpp
    policy/policy.cpp
    policy/pg.cpp
    testing.cpp
)
target_precompile_headers(${CURRENT_DIR_NAME}-testing PRIVATE prec.h)
target_link_libraries(${CURRENT_DIR_NAME}-testing utils mujoco libtorch python3)
