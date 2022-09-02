add_library(utils utils.cpp)
target_link_libraries(utils PUBLIC
    CONAN_PKG::spdlog
    deps_expected-lite
)

add_subdirectory(py3)
add_subdirectory(qt5)
add_subdirectory(gym)
add_subdirectory(parser)
add_subdirectory(clrs)
