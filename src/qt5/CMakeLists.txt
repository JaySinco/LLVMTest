get_filename_component(CURRENT_DIR_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)

find_package(Qt5Widgets CONFIG REQUIRED)

add_executable(${CURRENT_DIR_NAME}-testing
    find-dialog.cpp
    go-to-cell-dialog.cpp
    testing.cpp
)

set_target_properties(${CURRENT_DIR_NAME}-testing PROPERTIES
    AUTOMOC ON
    AUTORCC ON
    AUTOUIC ON
)

target_link_libraries(${CURRENT_DIR_NAME}-testing utils Qt5::Widgets)
