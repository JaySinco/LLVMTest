cmake_minimum_required(VERSION 3.7)
project(mongoose LANGUAGES C)

find_package(OpenSSL REQUIRED CONFIG)

add_library(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/mongoose.c)
set_target_properties(${PROJECT_NAME} PROPERTIES C_VISIBILITY_PRESET hidden)
target_link_libraries(${PROJECT_NAME} OpenSSL::Crypto OpenSSL::SSL)
target_compile_definitions(${PROJECT_NAME} PRIVATE MG_ENABLE_OPENSSL=1)

include(GNUInstallDirs)
install(TARGETS ${PROJECT_NAME}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(FILES ${CMAKE_SOURCE_DIR}/mongoose.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
