get_filename_component(CURRENT_DIR_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)

add_executable(uber-shader uber-shader.cpp)
target_link_libraries(uber-shader utils raylib argparse::argparse)

add_executable(pixel-shader pixel-shader.cpp)
target_link_libraries(pixel-shader utils raylib argparse::argparse)
