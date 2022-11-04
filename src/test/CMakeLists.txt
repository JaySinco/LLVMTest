get_filename_component(CURRENT_DIR_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
set(TEST ${CURRENT_DIR_NAME})

add_executable(${TEST}-clrs-sort clrs-sort.cpp)
target_link_libraries(${TEST}-clrs-sort utils Catch2::Catch2WithMain)

add_executable(${TEST}-clrs-tree clrs-tree.cpp)
target_link_libraries(${TEST}-clrs-tree utils Catch2::Catch2WithMain)

add_executable(${TEST}-clrs-dp clrs-dp.cpp)
target_link_libraries(${TEST}-clrs-dp utils Catch2::Catch2WithMain range-v3::range-v3 Boost::headers)

add_executable(${TEST}-cpp20 cpp20.cpp)
target_link_libraries(${TEST}-cpp20 utils range-v3::range-v3 Boost::headers)
set_property(TARGET ${TEST}-cpp20 PROPERTY CXX_STANDARD 20)

add_executable(${TEST}-https-server https-server.cpp)
target_link_libraries(${TEST}-https-server utils uWebSockets)

add_executable(${TEST}-concurrent-queue concurrent-queue.cpp)
target_link_libraries(${TEST}-concurrent-queue utils concurrentqueue)

add_executable(${TEST}-thread-pool thread-pool.cpp)
target_link_libraries(${TEST}-thread-pool utils threadpool)