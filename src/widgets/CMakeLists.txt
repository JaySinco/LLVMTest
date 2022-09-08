get_filename_component(CURRENT_DIR_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
set(TESTING ${CURRENT_DIR_NAME}-testing)

add_executable(${TESTING}
    testing-widget.cpp
    testing.cpp
)
set_target_properties(${TESTING} PROPERTIES
    AUTOMOC ON
    AUTORCC ON
    AUTOUIC ON
)
target_link_libraries(${TESTING} utils Qt5::Widgets Boost::timer)
