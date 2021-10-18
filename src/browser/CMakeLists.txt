add_executable(browser
    browser.cpp
    main.cpp
)

target_link_libraries(browser
    utils
    version.lib
    ${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib/opencv_world.lib
    ${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib/WebView2Loader.dll.lib
)

set_target_properties(browser PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/browser"
)
