add_library(python INTERFACE)

target_include_directories(python INTERFACE
    ${CMAKE_SOURCE_DIR}/deps/python/include/
    ${CMAKE_SOURCE_DIR}/deps/pybind11/include/
)

target_link_libraries(python INTERFACE
    ${CMAKE_SOURCE_DIR}/deps/python/lib/python38.lib
)

set(PYTHON_TARGET_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/python)

if(NOT EXISTS ${PYTHON_TARGET_DIR}/python.exe)
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/deps/python/bin/
            ${PYTHON_TARGET_DIR}/
        COMMAND_ERROR_IS_FATAL ANY
    )
    execute_process(
        COMMAND ${PYTHON_TARGET_DIR}/python.exe
            ${CMAKE_SOURCE_DIR}/resources/python/get-pip.py
        COMMAND_ERROR_IS_FATAL ANY
    )
    execute_process(
        COMMAND ${PYTHON_TARGET_DIR}/Scripts/pip.exe
            config set global.index-url https://pypi.tuna.tsinghua.edu.cn/simple
        COMMAND_ERROR_IS_FATAL ANY
    )
endif()

add_executable(testing-python testing.cpp)

target_link_libraries(testing-python PUBLIC utils python)

set_target_properties(testing-python PROPERTIES
    OUTPUT_NAME "testing"
    RUNTIME_OUTPUT_DIRECTORY ${PYTHON_TARGET_DIR}
)
