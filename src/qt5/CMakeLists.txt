get_filename_component(CURRENT_DIR_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
set(TESTING_EXE ${CURRENT_DIR_NAME}-testing)

find_package(Qt5Widgets CONFIG REQUIRED)

add_executable(${TESTING_EXE}
    find-dialog.cpp
    go-to-cell-dialog.cpp
    testing.cpp
)
set_target_properties(${TESTING_EXE} PROPERTIES
    AUTOMOC ON
    AUTORCC ON
    AUTOUIC ON
)
target_link_libraries(${TESTING_EXE} utils Qt5::Widgets)
