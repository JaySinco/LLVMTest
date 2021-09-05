add_executable(browser 
    browser.cpp
    main.cpp
)

target_include_directories(browser PUBLIC
    ${CMAKE_SOURCE_DIR}/deps/webview2/include/
    ${CMAKE_SOURCE_DIR}/deps/wil/include/
    ${CMAKE_SOURCE_DIR}/deps/json/include/
    ${CMAKE_SOURCE_DIR}/deps/opencv/include/
)

target_link_libraries(browser PUBLIC utils version.lib
    ${CMAKE_SOURCE_DIR}/deps/webview2/lib/WebView2LoaderStatic.lib
    ${CMAKE_SOURCE_DIR}/deps/opencv/x64/vc16/lib/opencv_world453.lib
)

add_custom_command(TARGET browser POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/deps/opencv/x64/vc16/bin/opencv_world453.dll
    $<TARGET_FILE_DIR:browser>
)

set_target_properties(browser PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/browser"
)
