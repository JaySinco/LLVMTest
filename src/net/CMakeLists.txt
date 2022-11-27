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

single_file_executable(tools-lsdev ${MAIN})
single_file_executable(tools-arp ${MAIN})

add_executable(${MAIN}-sniffer
    sniffer/hex-view.cpp
    sniffer/packets-acquirer.cpp
    sniffer/packets-model.cpp
    sniffer/packets-view.cpp
    sniffer/protocol-prop-view.cpp
    sniffer/top-panel.cpp
    sniffer/bottom-panel.cpp
    sniffer/main-window.cpp
    sniffer/main.cpp
)
target_include_directories(${MAIN}-sniffer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
set_target_properties(${MAIN}-sniffer PROPERTIES AUTOMOC ON AUTORCC ON AUTOUIC ON)
target_link_libraries(${MAIN}-sniffer ${MAIN} Qt5::Widgets)