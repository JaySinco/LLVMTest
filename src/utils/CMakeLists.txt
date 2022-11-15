get_filename_component(CURRENT_DIR_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
set(MAIN ${CURRENT_DIR_NAME})

add_library(${MAIN} base.cpp)

target_compile_definitions(${MAIN} PRIVATE -D_SOURCE_REPO="${SOURCE_REPO}")
exec_program("git" ${PROJECT_SOURCE_DIR} ARGS "rev-parse HEAD" OUTPUT_VARIABLE GIT_HASH)
target_compile_definitions(${MAIN} PRIVATE -D_GIT_HASH="${GIT_HASH}")
exec_program("git" ${PROJECT_SOURCE_DIR} ARGS "rev-parse --abbrev-ref HEAD" OUTPUT_VARIABLE GIT_BRANCH)
target_compile_definitions(${MAIN} PRIVATE -D_GIT_BRANCH="${GIT_BRANCH}")

target_link_libraries(${MAIN} PUBLIC
    spdlog::spdlog
    nonstd::expected-lite
    Iconv::Iconv
)
