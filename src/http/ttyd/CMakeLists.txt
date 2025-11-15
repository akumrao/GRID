cmake_minimum_required(VERSION 3.12.0)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

project(ttyd VERSION 1.7.7 LANGUAGES CXX)

set(TTYD_VERSION "${PROJECT_VERSION}")


set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

set(SOURCE_FILES src/utils.cpp src/pty.cpp src/protocol.cpp src/server.cpp)


file(GLOB src_HTTP

    ../src/url.cpp
    ../src/response.cpp
    ../src/request.cpp
    ../src/parser.cpp
    ../src/message.cpp
    ../src/httputil.cpp
    ../src/HttpConn.cpp
    ../src/cookie.cpp
    ../src/packetizers.cpp
    ../src/HTTPResponder.cpp
    ../src/form.cpp
    ../src/HttpServer.cpp
    ../src/client.cpp
    ../src/HttpClient.cpp
    ../src/websocket.cpp
    ../../base/src/*.cpp
    ../../net/src/T*.cpp
    ../../net/src/IP.cpp
    ../../net/src/dns.cpp
#    ../../net/uv_multiplex/src/*.cpp
    ../../http_parser/*.cpp
)





set(INCLUDE_DIRS  ../include/ ../../net/include/ ../../base/include ../../http_parser/ ../../json/include
  ../../libuv/include ../../libuv/src
)


set (CMAKE_REQUIRED_INCLUDES ${INCLUDE_DIRS})

if(UNIX)
list(APPEND LINK_LIBS dl Threads::Threads )
endif()

if(WIN32)
    list(APPEND LINK_LIBS shell32 ws2_32)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/app.rc.in ${CMAKE_CURRENT_BINARY_DIR}/app.rc @ONLY)
    list(APPEND SOURCE_FILES ${CMAKE_CURRENT_BINARY_DIR}/app.rc)
else()
    find_library(LIBUTIL NAMES util)
    if(LIBUTIL)
        list(APPEND LINK_LIBS util)
    endif()
endif()

add_subdirectory(../../libuv build )

add_executable(${PROJECT_NAME} ${SOURCE_FILES} ${src_HTTP})
target_include_directories(${PROJECT_NAME} PUBLIC ${INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${LINK_LIBS} uv_a)
target_compile_definitions(${PROJECT_NAME} PUBLIC
    TTYD_VERSION="${TTYD_VERSION}"
    $<$<PLATFORM_ID:Windows>:_WIN32_WINNT=0xa00 WINVER=0xa00>
)


if(UNIX)
target_compile_options(${PROJECT_NAME} PRIVATE -fpermissive -Wno-return-type -Wno-format-overflow -Wno-format -Wno-unused-but-set-variable -Wno-write-strings -no-pie)
endif()

if(WIN32)
target_compile_options(${PROJECT_NAME} PRIVATE )
endif()

include(GNUInstallDirs)

install(TARGETS ${PROJECT_NAME} DESTINATION "${CMAKE_INSTALL_BINDIR}" COMPONENT prog)
install(FILES man/ttyd.1 DESTINATION "${CMAKE_INSTALL_MANDIR}/man1" COMPONENT doc)
