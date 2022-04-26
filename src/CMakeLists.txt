add_library(utils utils.cpp)
target_compile_options(utils PUBLIC -Wno-ignored-attributes)
target_link_libraries(utils PUBLIC deps_fmt deps_gflags deps_glog)

add_executable(testing testing.cpp)
target_link_libraries(testing utils)

add_subdirectory(py3)
add_subdirectory(qt5)
add_subdirectory(gym)
