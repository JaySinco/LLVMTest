get_filename_component(CURRENT_DIR_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
set(MAIN ${CURRENT_DIR_NAME})

add_executable(${MAIN}-clrs-sort clrs-sort.cpp)
target_link_libraries(${MAIN}-clrs-sort utils Catch2::Catch2WithMain)

add_executable(${MAIN}-clrs-tree clrs-tree.cpp)
target_link_libraries(${MAIN}-clrs-tree utils Catch2::Catch2WithMain)

add_executable(${MAIN}-clrs-dp clrs-dp.cpp)
target_link_libraries(${MAIN}-clrs-dp utils Catch2::Catch2WithMain range-v3::range-v3 Boost::headers)

add_executable(${MAIN}-cpp20 cpp20.cpp)
target_link_libraries(${MAIN}-cpp20 utils range-v3::range-v3 Boost::headers)
set_property(TARGET ${MAIN}-cpp20 PROPERTY CXX_STANDARD 20)

add_executable(${MAIN}-uws uws.cpp)
target_link_libraries(${MAIN}-uws utils uWebSockets)
