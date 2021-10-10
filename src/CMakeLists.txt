add_library(utils utils.cpp)

target_include_directories(utils PUBLIC
    ${CMAKE_SOURCE_DIR}/deps/gflags/include/
    ${CMAKE_SOURCE_DIR}/deps/glog/include/
    ${CMAKE_SOURCE_DIR}/deps/fmt/include/
    ${CMAKE_SOURCE_DIR}/deps/boost/include/boost-1_76/
)

target_link_libraries(utils PUBLIC
    ${CMAKE_SOURCE_DIR}/deps/gflags/lib/gflags_static.lib
    ${CMAKE_SOURCE_DIR}/deps/glog/lib/glog.lib
    ${CMAKE_SOURCE_DIR}/deps/fmt/lib/fmt.lib
    shlwapi.lib
)

add_executable(testing-utils testing.cpp)

target_link_libraries(testing-utils PUBLIC utils)

set_target_properties(testing-utils PROPERTIES OUTPUT_NAME "testing")

add_subdirectory(python)
add_subdirectory(calc)
add_subdirectory(deep)
add_subdirectory(browser)
