get_filename_component(CURRENT_DIR_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
set(TESTING ${CURRENT_DIR_NAME}-testing)

add_executable(${TESTING}
    sort.cpp
    testing.cpp
)
target_link_libraries(${TESTING} utils Catch2::Catch2WithMain)
