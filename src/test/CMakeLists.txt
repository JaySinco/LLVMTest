get_filename_component(CURRENT_DIR_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
set(MAIN ${CURRENT_DIR_NAME})

function(add_test_executable)
    set(MAIN_TEST ${MAIN}-${ARGV0})
    add_executable(${MAIN_TEST} ${ARGV0}.cpp)
    math(EXPR LIBS_ARGC "${ARGC}-1")
    list(SUBLIST ARGN 1 ${LIBS_ARGC} DEP_LIBS)
    target_link_libraries(${MAIN_TEST} ${DEP_LIBS})
endfunction()

add_test_executable(clrs-sort utils Catch2::Catch2WithMain)
add_test_executable(clrs-tree utils Catch2::Catch2WithMain)
add_test_executable(clrs-dp utils Catch2::Catch2WithMain range-v3::range-v3 Boost::headers)

add_test_executable(cpp20 utils range-v3::range-v3 Boost::headers)
set_property(TARGET ${MAIN}-cpp20 PROPERTY CXX_STANDARD 20)

add_test_executable(https-server utils uWebSockets)
add_test_executable(concurrent-queue utils concurrentqueue)
add_test_executable(thread-pool utils threadpool)
