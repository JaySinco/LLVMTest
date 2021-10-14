add_library(utils utils.cpp)

find_package(fmt CONFIG REQUIRED)
find_package(glog CONFIG REQUIRED)

target_include_directories(utils PUBLIC
    ${CMAKE_SOURCE_DIR}/deps/gflags/include/
    ${CMAKE_SOURCE_DIR}/deps/glog/include/
    ${CMAKE_SOURCE_DIR}/deps/boost/include/boost-1_76/
)

target_link_libraries(utils PUBLIC
    fmt::fmt
    glog::glog
)

add_executable(testing-utils testing.cpp)

target_link_libraries(testing-utils PUBLIC utils)

set_target_properties(testing-utils PROPERTIES OUTPUT_NAME "testing")

add_subdirectory(python)
add_subdirectory(calc)
add_subdirectory(deep)
add_subdirectory(browser)
