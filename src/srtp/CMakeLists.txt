cmake_minimum_required(VERSION 3.12.0)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

project(runSrtp VERSION 1.7.7 LANGUAGES CXX)


set(TTYD_VERSION "${PROJECT_VERSION}")

set(CMAKE_CXX_STANDARD 17)

# Enforce the standard; if the compiler is too old, the build fails
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# set(CMAKE_C_STANDARD 17)
# set(CMAKE_C_STANDARD_REQUIRED ON)

# Options
option(USE_MBEDTLS "Use Mbed TLS instead of OpenSSL" OFF)



message("Current Source Directory: ${CMAKE_CURRENT_SOURCE_DIR}/../openssl/buildx64")
message("Current Binary Directory: ${CMAKE_CURRENT_BINARY_DIR}")
message("Current List File Directory: ${CMAKE_CURRENT_LIST_DIR}/../openssl/buildx64")


SET(SSL_ROOT "${CMAKE_CURRENT_LIST_DIR}/../openssl/buildx64")
SET(SSL_LIB "${CMAKE_CURRENT_LIST_DIR}/../openssl/buildx64/lib")




if (ENABLE_OPENSSL AND USE_MBEDTLS)
	message(FATAL_ERROR "Both USE_MBEDTLS and ENABLE_OPENSSL cannot be enabled at the same time")
endif()

if (NOT ENABLE_OPENSSL AND NOT USE_MBEDTLS)
	message(FATAL_ERROR "atleast one  USE_MBEDTLS or ENABLE_OPENSSL be enabled")
endif()



list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/Modules)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)


if(WIN32)
	add_definitions(-DWIN32_LEAN_AND_MEAN)
	if(MSVC)
		add_definitions(-DNOMINMAX)
		add_definitions(-D_CRT_SECURE_NO_WARNINGS)
		add_compile_definitions(_WIN32_WINNT=0x0A00)
		add_compile_options(/W2)
	endif()

else()
add_definitions( -Wno-write-strings -Wno-missing-field-initializers -Wno-unused-parameter -Wno-unused-value   -Wno-extra -Wno-switch)
endif()

if(CMAKE_COMPILER_IS_GNUCC)
    add_compile_options(-Wno-unused-but-set-variable)
endif()



set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)


# Configure usrsctp options

option(sctp_build_shared_lib OFF)
option(sctp_build_programs OFF)
option(sctp_inet OFF)
option(sctp_inet6 OFF)
option(sctp_werror OFF)
#set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)


set(sctp_build_shared_lib OFF CACHE BOOL "Build static" FORCE)
set(sctp_build_programs   OFF CACHE BOOL "Disable tools" FORCE)
set(sctp_werror           OFF CACHE BOOL "Disable werror" FORCE)

# Subdirectory inclusion
#add_subdirectory(usrsctp)
#add_subdirectory(./usrsctp buildusrstcp EXCLUDE_FROM_ALL)
add_subdirectory(${CMAKE_SOURCE_DIR}/../webrtcserver/deps/usrsctp buildusrstcp EXCLUDE_FROM_ALL)


if (MSYS OR MINGW)
	add_compile_definitions(${CMAKE_SOURCE_DIR}/../webrtcserver/deps/usrsctp PUBLIC -DSCTP_STDINT_INCLUDE=<stdint.h>)
endif()
#add_library(Usrsctp::Usrsctp ALIAS usrsctp)


link_libraries(usrsctp)



if(WIN32)
	link_libraries(  ws2_32) # winsock2
endif()


set(SOURCE_FILES main.cpp helpers.cpp)

if(UNIX)
file(GLOB SOURCES   
   src/*.cpp src/RTCP/*.cpp src/Codecs/*.cpp src/RtpDictionaries/*.cpp ../net/src/*.cpp ../net/dtls/*.cpp ../net/sctp/*.cpp ../net/pc/*.cpp ../json/src/*.cpp ../http/src/*.cpp ../http_parser/*.cpp ../base/src/*.cpp ../signal/src/*.cpp ../libuv/src/*.cpp ../libuv/src/unix/*.cpp
  )
else()
file(GLOB SOURCES   
    ../src/*.cpp ../dtls/*.cpp ../sctp/*.cpp ../pc/*.cpp ../../json/src/*.cpp ../../http/src/*.cpp ../../http_parser/*.cpp ../../base/src/*.cpp ../../signal/src/*.cpp 
  )
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

if(WIN32)
add_subdirectory(../../libuv build )
endif()


if(WIN32)
list(APPEND LINK_LIBS uv_a  )
endif()

if(UNIX)
list(APPEND LINK_LIBS  dl Threads::Threads )
endif()





if(USE_MBEDTLS) 
include_directories(../net/incmbedtls/)
endif()   


if(USE_MBEDTLS)
	#if(NOT TARGET MbedTLS::MbedTLS)
	#    set(MbedTLS_DEBUG 1)
	#	find_package(MbedTLS REQUIRED)
	#endif()

 
 	option(ENABLE_TESTING OFF)
	option(ENABLE_PROGRAMS OFF)
	set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)

	add_subdirectory(${CMAKE_SOURCE_DIR}/../webrtcserver/deps/mbedtls buildtmpmbed EXCLUDE_FROM_ALL)

	add_compile_definitions(  USE_MBEDTLS=1 HTTPSSL=1)

	include_directories(${CMAKE_SOURCE_DIR}/../webrtcserver/deps/mbedtls/include)
	link_libraries(   ${LINK_LIBS}  mbedtls mbedx509 mbedcrypto )
	
else()
	if(APPLE)
		# This is a bug in CMake that causes it to prefer the system version over
		# the one in the specified ROOT folder
		if(EXISTS ${OPENSSL_ROOT_DIR})
            # Use static files when OPENSSL_USE_STATIC_LIBS is set.
            # OPENSSL_USE_STATIC_LIBS is what CMake's FindOpenSSL looks at
            # to decide whether to use static libraries.
            if(OPENSSL_USE_STATIC_LIBS)
                set(OPENSSL_CRYPTO_LIBRARY "${OPENSSL_ROOT_DIR}/lib/libcrypto.a" CACHE FILEPATH "" FORCE)
                set(OPENSSL_SSL_LIBRARY "${OPENSSL_ROOT_DIR}/lib/libssl.a" CACHE FILEPATH "" FORCE)
            else()
                set(OPENSSL_CRYPTO_LIBRARY "${OPENSSL_ROOT_DIR}/lib/libcrypto.dylib" CACHE FILEPATH "" FORCE)
                set(OPENSSL_SSL_LIBRARY "${OPENSSL_ROOT_DIR}/lib/libssl.dylib" CACHE FILEPATH "" FORCE)
            endif()
		endif()
	endif()
	find_package(OpenSSL REQUIRED)
	add_compile_definitions(  HTTPSSL=1)
	#link_libraries(${SSL_LIB}/libssl.a ${SSL_LIB}/libcrypto.a)
	link_libraries(  ${LINK_LIBS} ${SSL_LIB}/libssl.a ${SSL_LIB}/libcrypto.a )
endif()

add_subdirectory(${CMAKE_SOURCE_DIR}/../webrtcserver/deps/libsrtp buildsrtp EXCLUDE_FROM_ALL )
add_subdirectory(deps/libwebrtc)


if(CAPI_STDCALL)
	add_compile_definitions( CAPI_STDCALL)
endif()


if(UNIX)
add_compile_options( -fpermissive -Wno-return-type -Wno-format-overflow -Wno-format -Wno-write-strings -no-pie)
endif()

if(WIN32)
#add_compile_options( PRIVATE )
endif()



if(NOT MSVC)
	add_compile_options( -Wall -Wextra)
endif()

if(WARNINGS_AS_ERRORS)
	if(MSVC)
		add_compile_options(  /WX)
	else()
		add_compile_options(  -Werror)
	endif()
endif()



include_directories(
           ./include ../net/sctp/ ../net/pc/ ../net/dtls/ ../http/include/ ../http_parser/ ../base/include/ ../net/include/ ../libuv/include ../libuv/src ../json/include/ ../json/include/json ../http_parser/ ../signal/include/ ../../src/openssl/buildx64/include ../webrtcserver/deps/usrsctp/usrsctplib/ ../webrtcserver/deps/libsrtp/include  ./deps/libwebrtc  ./deps/libwebrtc/libwebrtc ./deps/libwebrtc/abseil-cpp ${SSL_ROOT}/include)



add_executable(${PROJECT_NAME} ${SOURCE_FILES} ${SOURCES})
target_include_directories(${PROJECT_NAME} PUBLIC ${INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE webrtc srtp2)
target_compile_definitions(${PROJECT_NAME} PUBLIC DUALSTACK SRTP RTC_ENABLE_MEDIA MS_LITTLE_ENDIAN TTYD_VERSION="${TTYD_VERSION}"
    $<$<PLATFORM_ID:Windows>:_WIN32_WINNT=0xa00 WINVER=0xa00>
)


