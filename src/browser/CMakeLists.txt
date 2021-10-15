add_executable(browser 
    browser.cpp
    main.cpp
)

target_include_directories(browser PRIVATE
    ${CMAKE_SOURCE_DIR}/deps/webview2/include/
)

target_link_libraries(browser PRIVATE
    utils
    version.lib
    ${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib/opencv_world.lib
    ${CMAKE_SOURCE_DIR}/deps/webview2/lib/WebView2LoaderStatic.lib
)

set_target_properties(browser PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/browser"
)
