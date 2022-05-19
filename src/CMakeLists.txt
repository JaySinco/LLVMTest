add_library(utils utils.cpp)
target_link_libraries(utils PUBLIC
    deps_fmt
    deps_gflags
    deps_glog
    deps_expected-lite
)

add_executable(testing testing.cpp)
target_link_libraries(testing utils deps_boost)

add_subdirectory(py3)
add_subdirectory(qt5)
add_subdirectory(gym)
add_subdirectory(parser)
