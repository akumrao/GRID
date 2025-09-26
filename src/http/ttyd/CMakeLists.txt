cmake_minimum_required(VERSION 3.12.0)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

project(ttyd VERSION 1.7.7 LANGUAGES CXX)

set(TTYD_VERSION "${PROJECT_VERSION}")


set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

set(SOURCE_FILES src/utils.cpp src/pty.cpp src/protocol.cpp src/http.cpp src/server.cpp)

#set(WORKSPACE "/usr/local/google/home/aumrao/workspace/MagicAi")
SET(SSL_ROOT "${WORKSPACE}/src/openssl/buildx64")
SET(SSL_LIB "${WORKSPACE}/src/openssl/buildx64/lib")


file(GLOB src_HTTP
    ../src/*.cpp
    ../../libuv/src/*.cpp
    ../../libuv/src/unix/*.cpp
    ../../base/src/*.cpp
    ../../crypto/src/*.cpp
    ../../net/src/*.cpp
    ../../net/uv_multiplex/src/*.cpp
    ../../http_parser/*.cpp
)




find_package(ZLIB REQUIRED)
#find_package(Libwebsockets 3.2.0 REQUIRED)

set(INCLUDE_DIRS ${ZLIB_INCLUDE_DIR} ${LIBWEBSOCKETS_INCLUDE_DIRS}  ../include/ ../../net/include/ ../../base/include ../../http_parser/ ../../json/include
 ../../net/uv_multiplex/include/ ../../libuv/include ../../libuv/src ${SSL_ROOT}/include
)

set(LINK_LIBS ${ZLIB_LIBRARIES} ${LIBWEBSOCKETS_LIBRARIES}  )

set (CMAKE_REQUIRED_INCLUDES ${INCLUDE_DIRS})
include(CheckSymbolExists)
#check_symbol_exists(LWS_WITH_LIBUV "lws_config.h" LWS_WITH_LIBUV)
#check_symbol_exists(LWS_OPENSSL_SUPPORT "lws_config.h" LWS_OPENSSL_ENABLED)
#check_symbol_exists(LWS_WITH_MBEDTLS "lws_config.h" LWS_MBEDTLS_ENABLED)
#if(NOT LWS_WITH_LIBUV)
#    message(FATAL_ERROR "libwebsockets was not build with libuv support (-DLWS_WITH_LIBUV=ON)")
#endif()

list(APPEND LINK_LIBS dl Threads::Threads ${SSL_LIB}/libssl.a ${SSL_LIB}/libcrypto.a)


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

add_executable(${PROJECT_NAME} ${SOURCE_FILES} ${src_HTTP})
target_include_directories(${PROJECT_NAME} PUBLIC ${INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${LINK_LIBS})
target_compile_definitions(${PROJECT_NAME} PUBLIC
    TTYD_VERSION="${TTYD_VERSION}"
    $<$<PLATFORM_ID:Windows>:_WIN32_WINNT=0xa00 WINVER=0xa00>
)

target_compile_options(${PROJECT_NAME} PRIVATE -fpermissive -Wno-return-type -Wno-format-overflow -Wno-format -Wno-unused-but-set-variable -Wno-write-strings -no-pie)

include(GNUInstallDirs)

install(TARGETS ${PROJECT_NAME} DESTINATION "${CMAKE_INSTALL_BINDIR}" COMPONENT prog)
install(FILES man/ttyd.1 DESTINATION "${CMAKE_INSTALL_MANDIR}/man1" COMPONENT doc)
