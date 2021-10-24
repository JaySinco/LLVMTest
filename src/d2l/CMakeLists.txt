get_filename_component(CURRENT_DIR ${CMAKE_CURRENT_SOURCE_DIR} NAME)

add_library(libtorch INTERFACE)

target_include_directories(libtorch INTERFACE
    ${TORCH_ROOT_DIR}/include/torch/csrc/api/include/
)

target_link_libraries(libtorch INTERFACE ${TORCH_LIBRARIES})

if(NOT EXISTS ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/torch.dll)
    file(COPY ${TORCH_ROOT_DIR}/lib/
        DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
        FILES_MATCHING PATTERN "*.dll"
    )
endif()

add_executable(linear-regression linear-regression.cpp)
target_precompile_headers(linear-regression PRIVATE prec.h)
target_link_libraries(linear-regression utils libtorch)

add_executable(fashion-mnist fashion-mnist.cpp)
target_precompile_headers(fashion-mnist REUSE_FROM linear-regression)
target_link_libraries(fashion-mnist utils python3 libtorch)
