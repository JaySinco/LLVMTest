get_filename_component(CURRENT_DIR_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5Widgets CONFIG REQUIRED)

add_executable(${CURRENT_DIR_NAME}-testing
    find-dialog.cpp
    testing.cpp
)

target_link_libraries(${CURRENT_DIR_NAME}-testing utils Qt5::Widgets)
