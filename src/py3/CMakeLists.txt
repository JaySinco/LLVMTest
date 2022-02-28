get_filename_component(CURRENT_DIR_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)

add_library(${CURRENT_DIR_NAME} INTERFACE)

target_include_directories(${CURRENT_DIR_NAME} INTERFACE
    ${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include/python3
)

target_link_libraries(${CURRENT_DIR_NAME} INTERFACE
    ${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib/python38.lib
)

set(PYTHON_TARGET_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CURRENT_DIR_NAME})

if(NOT EXISTS ${PYTHON_TARGET_DIR}/python.exe)
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/etc/python3/
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

add_executable(${CURRENT_DIR_NAME}-testing testing.cpp)
target_link_libraries(${CURRENT_DIR_NAME}-testing utils ${CURRENT_DIR_NAME})
