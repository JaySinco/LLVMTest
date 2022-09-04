find_package(spdlog REQUIRED)
find_package(Boost REQUIRED)
find_package(Qt5Widgets REQUIRED PATHS ${DEPS_PREFIX}/qt5)

add_subdirectory(utils)
add_subdirectory(gym)
add_subdirectory(parser)
add_subdirectory(widgets)
add_subdirectory(clrs)
