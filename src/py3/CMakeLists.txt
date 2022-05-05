get_filename_component(CURRENT_DIR_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
set(TESTING ${CURRENT_DIR_NAME}-testing)

add_executable(${TESTING} testing.cpp)
set_target_properties(${TESTING} PROPERTIES ENABLE_EXPORTS 1)
target_link_libraries(${TESTING} utils deps_pybind11)
set(PIP3 ${DEPS_PREFIX}/python3/bin/pip3)
add_custom_command(TARGET ${TESTING} POST_BUILD
    COMMAND ${PIP3} config set global.index-url https://pypi.tuna.tsinghua.edu.cn/simple
    COMMAND ${PIP3} install -q -r ${CMAKE_CURRENT_SOURCE_DIR}/requirements.txt
)
