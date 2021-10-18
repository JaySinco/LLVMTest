set(TORCH_ROOT_DIR ${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/tools/libtorch/)

find_package(Torch REQUIRED PATHS ${TORCH_ROOT_DIR}/share/cmake/)

add_executable(deep
    linear-regression.cpp
    fashion-mnist.cpp
    main.cpp
)

target_precompile_headers(deep PRIVATE prec.h)

target_include_directories(deep PRIVATE
    ${TORCH_ROOT_DIR}}/include/
    ${TORCH_ROOT_DIR}}/include/torch/csrc/api/include/
)

target_link_libraries(deep
    utils
    python
    ${TORCH_LIBRARIES}
    ${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib/opencv_world.lib
)

set_target_properties(deep PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/deep"
)

file(WRITE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/deep/python38._pth
    "..\\python\\Lib\n..\\python\\Lib\\site-packages\n..\\python\\python38.zip\n.\n"
)

if(NOT EXISTS ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/deep/torch.dll)
    file(GLOB TORCH_DLLS ${TORCH_ROOT_DIR}/lib/*.dll)
    add_custom_command(TARGET deep POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${TORCH_DLLS}
        $<TARGET_FILE_DIR:deep>
    )
endif()
