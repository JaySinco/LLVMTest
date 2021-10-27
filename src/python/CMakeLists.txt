get_filename_component(CURRENT_DIR_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)

add_library(python3 INTERFACE)

target_include_directories(python3 INTERFACE
    ${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include/python3
)

target_link_libraries(python3 INTERFACE
    ${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib/python38.lib
)

set(PYTHON_TARGET_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CURRENT_DIR_NAME})

if(NOT EXISTS ${PYTHON_TARGET_DIR}/python.exe)
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/extras/python3/
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
    ".\\python\\Lib\n.\\python\\Lib\\site-packages\n.\\python\\python38.zip\n.\n"
)

add_executable(${CURRENT_DIR_NAME}-testing testing.cpp)
target_link_libraries(${CURRENT_DIR_NAME}-testing utils python3)
