set(TORCH_ROOT_DIR ${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/extras/libtorch/)
find_package(Torch REQUIRED PATHS ${TORCH_ROOT_DIR}/share/cmake/)

add_library(utils utils.cpp)

target_compile_options(utils PUBLIC -Wno-ignored-attributes)

target_include_directories(utils PUBLIC
    ${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include/
)

target_link_libraries(utils PUBLIC
    ${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib/fmt.lib
    ${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib/gflags.lib
    ${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib/glog.lib
)

add_executable(testing testing.cpp)

target_link_libraries(testing utils)

add_subdirectory(python)
add_subdirectory(d2l)
add_subdirectory(gym)
