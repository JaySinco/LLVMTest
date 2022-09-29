get_filename_component(CURRENT_DIR_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
set(MAIN ${CURRENT_DIR_NAME})

add_executable(${MAIN} main.cpp)
target_link_libraries(${MAIN} utils range-v3::range-v3)
# set_property(TARGET ${MAIN} PROPERTY CXX_STANDARD 20)