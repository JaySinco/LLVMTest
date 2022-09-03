find_package(spdlog REQUIRED)
find_package(Boost REQUIRED)

add_library(utils utils.cpp)
target_link_libraries(utils PUBLIC
    spdlog::spdlog
    deps_expected-lite
)

add_subdirectory(qt5)
add_subdirectory(gym)
add_subdirectory(parser)
add_subdirectory(clrs)
