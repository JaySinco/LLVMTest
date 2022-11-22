get_filename_component(CURRENT_DIR_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
set(MAIN ${CURRENT_DIR_NAME})

add_executable(${MAIN}
    base-shader.cpp
    uber-shader.cpp
    pixel-shader.cpp
    main.cpp
)

target_link_libraries(${MAIN} utils
    Boost::program_options
    nlohmann-json
    raylib
)
