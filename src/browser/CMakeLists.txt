find_package(nlohmann_json CONFIG REQUIRED)
find_package(WIL CONFIG REQUIRED)
find_package(OpenCV CONFIG REQUIRED)

add_executable(browser 
    browser.cpp
    main.cpp
)

target_include_directories(browser PUBLIC
    ${CMAKE_SOURCE_DIR}/deps/webview2/include/
)

target_link_libraries(browser PUBLIC
    utils
    version.lib
    WIL::WIL
    opencv_world
    nlohmann_json::nlohmann_json
    ${CMAKE_SOURCE_DIR}/deps/webview2/lib/WebView2LoaderStatic.lib
)

set_target_properties(browser PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/browser"
)
