get_filename_component(CURRENT_DIR_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
set(TESTING ${CURRENT_DIR_NAME}-testing)

add_executable(${TESTING} testing.cpp parser.cpp)
target_precompile_headers(${TESTING} PRIVATE prec.h)
target_link_libraries(${TESTING} utils deps_boost)
