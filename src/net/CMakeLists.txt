get_filename_component(CURRENT_DIR_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
set(MAIN ${CURRENT_DIR_NAME})

add_library(${MAIN}
    type.cpp
    platform_$<IF:$<BOOL:${WIN32}>,win32,linux>.cpp
    driver.cpp
    protocol.cpp
    ethernet.cpp
    arp.cpp
    ipv4.cpp
    icmp.cpp
    udp.cpp
    tcp.cpp
    dns.cpp
)
target_link_libraries(${MAIN} utils nlohmann-json Boost::headers)
if (WIN32)
    target_link_libraries(${MAIN} shlwapi ws2_32 iphlpapi npcap::npcap)
elseif (UNIX)
    target_link_libraries(${MAIN} shlwapi ws2_32 iphlpapi libpcap::libpcap)
endif()

single_file_executable(tools/lsdev ${MAIN})
single_file_executable(tools/arp ${MAIN} Boost::program_options)

add_executable(${MAIN}-tools-sniffer
    tools/sniffer-main-window.cpp
    tools/sniffer.cpp
)
set_target_properties(${MAIN}-tools-sniffer PROPERTIES AUTOMOC ON AUTORCC ON AUTOUIC ON)
target_link_libraries(${MAIN}-tools-sniffer utils Qt5::Widgets)