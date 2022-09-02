get_filename_component(CURRENT_DIR_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
set(TESTING ${CURRENT_DIR_NAME}-testing)

add_executable(${TESTING} testing.cpp)
target_precompile_headers(${TESTING} PRIVATE prec.h)
target_link_libraries(${TESTING} utils boost::headers)

add_executable(spirit-demo demo.cpp)
target_precompile_headers(spirit-demo REUSE_FROM ${TESTING})
target_link_libraries(spirit-demo utils boost::headers)
