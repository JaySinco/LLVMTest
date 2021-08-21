add_executable(deep
    main.cpp
)

target_precompile_headers(deep PUBLIC prec.h)

target_include_directories(deep PUBLIC
    ${CMAKE_SOURCE_DIR}/deps/libtorch/include/
    ${CMAKE_SOURCE_DIR}/deps/libtorch/include/torch/csrc/api/include/
)

target_link_libraries(deep PUBLIC
    ${CMAKE_SOURCE_DIR}/deps/libtorch/lib/c10.lib
    ${CMAKE_SOURCE_DIR}/deps/libtorch/lib/torch_cpu.lib
)

file(GLOB TORCH_DLLS ${CMAKE_SOURCE_DIR}/deps/libtorch/lib/*.dll)
add_custom_command(TARGET deep POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${TORCH_DLLS}
    $<TARGET_FILE_DIR:deep>
)
