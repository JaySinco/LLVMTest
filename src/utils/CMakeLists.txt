get_filename_component(CURRENT_DIR_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
set(MAIN ${CURRENT_DIR_NAME})

add_library(${MAIN} base.cpp)

string(CONCAT SOURCE_REPO "\"" ${SOURCE_REPO} "\"")
target_compile_definitions(${MAIN} PRIVATE SOURCE_REPO=${SOURCE_REPO})

target_link_libraries(${MAIN} PUBLIC
    spdlog::spdlog
    nonstd::expected-lite
    Iconv::Iconv
)
