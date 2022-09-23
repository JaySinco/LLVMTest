get_filename_component(CURRENT_DIR_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
set(MAIN ${CURRENT_DIR_NAME})
set(CMPL ${CURRENT_DIR_NAME}-autocomplete)

add_executable(${MAIN} main.cpp)
target_precompile_headers(${MAIN} PRIVATE prec.h)
target_link_libraries(${MAIN} utils Boost::headers)

add_executable(${CMPL} autocomplete.cpp)
target_precompile_headers(${CMPL} REUSE_FROM ${MAIN})
target_link_libraries(${CMPL} utils Boost::headers)
