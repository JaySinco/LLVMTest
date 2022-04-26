get_filename_component(CURRENT_DIR_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
set(PYTHON_TARGET_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CURRENT_DIR_NAME})
set(TESTING_EXE ${CURRENT_DIR_NAME}-testing)

if(NOT EXISTS ${PYTHON_TARGET_DIR}/python.exe)
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${PROTO_VCPKG_PREFIX}/etc/python3/
            ${PYTHON_TARGET_DIR}/
        COMMAND_ERROR_IS_FATAL ANY
    )
    execute_process(
        COMMAND ${PYTHON_TARGET_DIR}/python.exe
            ${CMAKE_CURRENT_SOURCE_DIR}/get-pip.py
        COMMAND_ERROR_IS_FATAL ANY
    )
    execute_process(
        COMMAND ${PYTHON_TARGET_DIR}/Scripts/pip.exe
            config set global.index-url https://pypi.tuna.tsinghua.edu.cn/simple
        COMMAND_ERROR_IS_FATAL ANY
    )
    execute_process(
        COMMAND ${PYTHON_TARGET_DIR}/Scripts/pip.exe install -r
            ${CMAKE_CURRENT_SOURCE_DIR}/requirements.txt
        COMMAND_ERROR_IS_FATAL ANY
    )
endif()

file(WRITE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/python38._pth
    ".\\${CURRENT_DIR_NAME}\\Lib\n"
    ".\\${CURRENT_DIR_NAME}\\Lib\\site-packages\n"
    ".\\${CURRENT_DIR_NAME}\\python38.zip\n"
    ".\n"
)

add_executable(${TESTING_EXE} testing.cpp)
target_link_libraries(${TESTING_EXE} utils deps_python3)
