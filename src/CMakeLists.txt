add_library(utils utils.cpp)

target_include_directories(utils PUBLIC
    ${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include/
)

target_link_libraries(utils PUBLIC
    ${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib/fmt.lib
    ${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib/gflags.lib
    ${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib/glog.lib
)

add_executable(testing-utils testing.cpp)

target_link_libraries(testing-utils PRIVATE utils)

set_target_properties(testing-utils PROPERTIES OUTPUT_NAME "testing")

add_subdirectory(python)
add_subdirectory(calc)
add_subdirectory(deep)
add_subdirectory(browser)
