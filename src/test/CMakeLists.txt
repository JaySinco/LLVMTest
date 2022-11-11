get_filename_component(CURRENT_DIR_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
set(MAIN ${CURRENT_DIR_NAME})

add_executable(${MAIN} main.cpp)
target_link_libraries(${MAIN} utils)

single_file_executable(cpp20 utils range-v3::range-v3 Boost::headers)
set_property(TARGET ${MAIN}-cpp20 PROPERTY CXX_STANDARD 20)

single_file_executable(clrs-sort utils Catch2::Catch2WithMain)
single_file_executable(clrs-tree utils Catch2::Catch2WithMain)
single_file_executable(clrs-dp utils Catch2::Catch2WithMain range-v3::range-v3 Boost::headers)
single_file_executable(https-server utils uWebSockets)
single_file_executable(concurrent-queue utils concurrentqueue)
single_file_executable(thread-pool utils threadpool)
