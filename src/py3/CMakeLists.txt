get_filename_component(CURRENT_DIR_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
set(TESTING_EXE ${CURRENT_DIR_NAME}-testing)

set(PIP3_EXE ${DEPS_PREFIX}/python3/bin/pip3)
execute_process(COMMAND ${PIP3_EXE} config set global.index-url https://pypi.tuna.tsinghua.edu.cn/simple)
execute_process(COMMAND ${PIP3_EXE} install -r ${CMAKE_CURRENT_SOURCE_DIR}/requirements.txt)

add_executable(${TESTING_EXE} testing.cpp)
target_link_libraries(${TESTING_EXE} utils deps_pybind11)
set_target_properties(${TESTING_EXE} PROPERTIES ENABLE_EXPORTS 1)
add_custom_command(TARGET ${TESTING_EXE} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${DEPS_PREFIX}/libffi/lib/libffi.so.8
    $<TARGET_FILE_DIR:${TESTING_EXE}>
)
