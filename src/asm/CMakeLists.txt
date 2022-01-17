get_filename_component(CURRENT_DIR_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)

add_executable(fixed-vhd-writer fixed-vhd-writer.cpp)
target_link_libraries(fixed-vhd-writer utils)
