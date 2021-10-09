find_package(Torch REQUIRED PATHS ${CMAKE_SOURCE_DIR}/deps/libtorch/share/cmake/)

add_executable(deep
    linear-regression.cpp
    fashion-mnist.cpp
    main.cpp
)
target_precompile_headers(deep PUBLIC prec.h)

target_include_directories(deep PUBLIC
    ${CMAKE_SOURCE_DIR}/deps/libtorch/include/
    ${CMAKE_SOURCE_DIR}/deps/libtorch/include/torch/csrc/api/include/
    ${CMAKE_SOURCE_DIR}/deps/opencv/include/
)

target_link_libraries(deep PUBLIC utils python ${TORCH_LIBRARIES}
    ${CMAKE_SOURCE_DIR}/deps/opencv/x64/vc16/lib/opencv_world453.lib
)

set_target_properties(deep PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/deep"
)

file(WRITE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/deep/python38._pth
    "..\\python\\Lib\n..\\python\\Lib\\python38.zip\n.\nimport site"
)

add_custom_command(TARGET deep POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/deps/opencv/x64/vc16/bin/opencv_world453.dll
        ${CMAKE_SOURCE_DIR}/deps/python/bin/python3.dll
        ${CMAKE_SOURCE_DIR}/deps/python/bin/python38.dll
    $<TARGET_FILE_DIR:deep>
)

if(NOT EXISTS ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/deep/torch.dll)
    file(GLOB TORCH_DLLS ${CMAKE_SOURCE_DIR}/deps/libtorch/lib/*.dll)
    add_custom_command(TARGET deep POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${TORCH_DLLS}
        $<TARGET_FILE_DIR:deep>
    )
endif()
