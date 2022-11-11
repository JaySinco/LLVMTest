get_filename_component(CURRENT_DIR_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
set(MAIN ${CURRENT_DIR_NAME})

add_library(${MAIN}
    type.cpp
    platform_$<IF:$<BOOL:${WIN32}>,win32,linux>.cpp
    protocol.cpp
    ethernet.cpp
    arp.cpp
    ipv4.cpp
)
target_link_libraries(${MAIN} utils nlohmann-json)
if (WIN32)
    target_link_libraries(${MAIN} shlwapi ws2_32 iphlpapi)
elseif (UNIX)
endif()

single_file_executable(tools-lsdev ${MAIN})