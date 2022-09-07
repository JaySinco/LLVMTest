get_filename_component(CURRENT_DIR_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
set(TESTING ${CURRENT_DIR_NAME}-testing)
set(CMPL ${CURRENT_DIR_NAME}-autocomplete)

add_executable(${TESTING} testing.cpp)
target_precompile_headers(${TESTING} PRIVATE prec.h)
target_link_libraries(${TESTING} utils Boost::headers Boost::timer)

add_executable(${CMPL} autocomplete.cpp)
target_precompile_headers(${CMPL} REUSE_FROM ${TESTING})
target_link_libraries(${CMPL} utils Boost::headers)
