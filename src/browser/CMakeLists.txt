add_executable(browser 
    browser.cpp
    main.cpp
)

target_include_directories(browser PUBLIC
    ${CMAKE_SOURCE_DIR}/deps/webview2/include/
    ${CMAKE_SOURCE_DIR}/deps/wil/include/
    ${CMAKE_SOURCE_DIR}/deps/json/include/
)

target_link_libraries(browser PUBLIC utils version.lib
    ${CMAKE_SOURCE_DIR}/deps/webview2/lib/WebView2LoaderStatic.lib
)

set_target_properties(browser PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/browser"
)
