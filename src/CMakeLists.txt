add_library(utils utils.cpp)
target_link_libraries(utils PUBLIC
    deps_fmt
    deps_spdlog
    deps_expected-lite
)

add_executable(testing testing.cpp)
target_link_libraries(testing utils deps_boost deps_catch2)

add_subdirectory(py3)
add_subdirectory(qt5)
add_subdirectory(gym)
add_subdirectory(parser)
