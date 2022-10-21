get_filename_component(CURRENT_DIR_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
set(LIB ${CURRENT_DIR_NAME})

add_library(${LIB} base.cpp)

string(CONCAT SOURCE_REPO "\"" ${SOURCE_REPO} "\"")
target_compile_definitions(${LIB} PRIVATE SOURCE_REPO=${SOURCE_REPO})

target_link_libraries(${LIB} PUBLIC
    spdlog::spdlog
    nonstd::expected-lite
    Iconv::Iconv
)
