add_library(python INTERFACE)

target_include_directories(python INTERFACE
    ${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include/python3
)

target_link_libraries(python INTERFACE
    ${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib/python38.lib
)

set(PYTHON_TARGET_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/python)

if(NOT EXISTS ${PYTHON_TARGET_DIR}/python.exe)
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/tools/python3/
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
    execute_process(
        COMMAND ${PYTHON_TARGET_DIR}/Scripts/pip.exe install -r
            ${CMAKE_SOURCE_DIR}/resources/python/requirements.txt
        COMMAND_ERROR_IS_FATAL ANY
    )
endif()

add_executable(testing-python testing.cpp)

target_link_libraries(testing-python PRIVATE utils python)

set_target_properties(testing-python PROPERTIES
    OUTPUT_NAME "testing"
    RUNTIME_OUTPUT_DIRECTORY ${PYTHON_TARGET_DIR}
)
