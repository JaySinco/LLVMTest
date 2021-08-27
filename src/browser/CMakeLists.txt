find_package(Torch REQUIRED PATHS ${CMAKE_SOURCE_DIR}/deps/libtorch/share/cmake/)

add_executable(browser WIN32
    simple-handler.cpp
    simple-app.cpp
    main.cpp
)

target_include_directories(browser PUBLIC
    ${CMAKE_SOURCE_DIR}/deps/libcef/
)

target_link_libraries(browser PUBLIC utils
    ${CMAKE_SOURCE_DIR}/deps/libcef/lib/libcef.lib
    ${CMAKE_SOURCE_DIR}/deps/libcef/lib/libcef_dll_wrapper.lib
)

set_target_properties(browser PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/browser"
)

if(NOT EXISTS ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/browser/libcef.dll)
    add_custom_command(TARGET browser POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/deps/libcef/bin/
        $<TARGET_FILE_DIR:browser>
    )
endif()
