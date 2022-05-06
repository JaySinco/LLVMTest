get_filename_component(CURRENT_DIR_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
set(TESTING ${CURRENT_DIR_NAME}-testing)

add_executable(${TESTING} testing.cpp)
target_link_libraries(${TESTING} utils deps_pybind11)
if(UNIX)
    set(PIP3 ${DEPS_PREFIX}/python3/bin/pip3)
    set_target_properties(${TESTING} PROPERTIES ENABLE_EXPORTS 1)
elseif(WIN32)
    set(PIP3 ${DEPS_PREFIX}/python3/bin/Scripts/pip3.exe)
    string(CONCAT DEPS_PYTHON_SYS_PATH
        "${DEPS_PREFIX}/python3/bin/DLLs\n"
        "${DEPS_PREFIX}/python3/bin/Lib\n"
        "${DEPS_PREFIX}/python3/bin/Lib/site-packages\n"
    )
    file(WRITE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/python38._pth ${DEPS_PYTHON_SYS_PATH})
    add_custom_command(TARGET ${TESTING} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${DEPS_PREFIX}/python3/bin/python38.dll
        $<TARGET_FILE_DIR:${TESTING}>
    )
endif()
add_custom_command(TARGET ${TESTING} POST_BUILD
    COMMAND ${PIP3} config set global.index-url https://pypi.tuna.tsinghua.edu.cn/simple
    COMMAND ${PIP3} install --no-warn-script-location -q -r ${CMAKE_CURRENT_SOURCE_DIR}/requirements.txt
)
