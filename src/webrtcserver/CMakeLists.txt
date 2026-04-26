
cmake_minimum_required(VERSION 3.12) # Or a more recent version
if(POLICY CMP0074)
    cmake_policy(SET CMP0074 NEW)
endif()


project(libdatachannel
	VERSION 0.21.2
	LANGUAGES CXX)
set(PROJECT_DESCRIPTION "C/C++ WebRTC network library featuring Data Channels, Media Transport, and WebSockets")

include(GNUInstallDirs)

# Options
option(USE_GNUTLS "Use GnuTLS instead of OpenSSL" OFF)
option(USE_MBEDTLS "Use Mbed TLS instead of OpenSSL" OFF)
#ption(USE_NICE "Use libnice instead of libjuice" OFF)
option(PREFER_SYSTEM_LIB "Prefer system libraries over deps folder" OFF)
option(USE_SYSTEM_SRTP "Use system libSRTP" ${PREFER_SYSTEM_LIB})
#option(USE_SYSTEM_JUICE "Use system libjuice" ${PREFER_SYSTEM_LIB})
option(USE_SYSTEM_USRSCTP "Use system libusrsctp" ${PREFER_SYSTEM_LIB})
option(NO_WEBSOCKET "Disable WebSocket support" OFF)
option(NO_MEDIA "Disable media transport support" OFF)
option(NO_EXAMPLES "Disable examples" OFF)
option(NO_TESTS "Disable tests build" OFF)
option(WARNINGS_AS_ERRORS "Treat warnings as errors" OFF)
option(CAPI_STDCALL "Set calling convention of C API callbacks stdcall" OFF)
option(SCTP_DEBUG "Enable SCTP debugging output to verbose log" OFF)
option(RTC_UPDATE_VERSION_HEADER "Enable updating the version header" OFF)


message("Current Source Directory: ${CMAKE_CURRENT_SOURCE_DIR}/../openssl/buildx64")
message("Current Binary Directory: ${CMAKE_CURRENT_BINARY_DIR}")
message("Current List File Directory: ${CMAKE_CURRENT_LIST_DIR}/../openssl/buildx64")


SET(SSL_ROOT "${CMAKE_CURRENT_LIST_DIR}/../openssl/buildx64")
SET(SSL_LIB "${CMAKE_CURRENT_LIST_DIR}/../openssl/buildx64/lib")


 
#set(MbedTLS_ROOT ${WORKSPACE}/src/webrtcserver/deps/mbedtls/)
#set(MbedTLS_DIR ${WORKSPACE}/src/mbedtls/build/)
#set(MBEDTLS_INCLUDE_DIRS ${WORKSPACE}/src/mbedtls/build/library)
#set(MBEDTLS_LIBRARIES ${WORKSPACE}/src/mbedtls/build/include)

#set(MbedTLS_LIBRARY ${WORKSPACE}/src/mbedtls/build/library)
#set(MbedTLS_INCLUDE_DIR ${WORKSPACE}/src/mbedtls/build/include)

#set(MbedCrypto_LIBRARY ${WORKSPACE}/src/mbedtls/build/library)
#set(MbedX509_LIBRARY ${WORKSPACE}/src/mbedtls/build/library)


# set(ENABLE_OPENSSL 1) make sure ssl or mbedtl is set otherwise it will not work
# set(USE_MBEDTLS 1) make sure ssl or mbedtl is set otherwise it will not work

if (ENABLE_OPENSSL AND USE_MBEDTLS)
	message(FATAL_ERROR "Both USE_MBEDTLS and USE_GNUTLS cannot be enabled at the same time")
endif()

if (NOT ENABLE_OPENSSL AND NOT USE_MBEDTLS)
	message(FATAL_ERROR "atleast one  USE_MBEDTLS or USE_GNUTLS be enabled")
endif()


if(USE_GNUTLS)
#	option(USE_NETTLE "Use Nettle in libjuice" ON)
#else()
#	option(USE_NETTLE "Use Nettle in libjuice" OFF)

	if(NOT USE_SYSTEM_SRTP)
		if (USE_MBEDTLS)
			option(ENABLE_MBEDTLS "Enable Mbed TLS crypto engine for libSRTP" ON)
		else()
			option(ENABLE_OPENSSL "Enable OpenSSL crypto engine for libSRTP" ON)
		endif()
	endif()
endif()

if(NOT WARNINGS_AS_ERRORS AND NOT USE_SYSTEM_SRTP)
	option(BUILD_WITH_WARNINGS "Add warning as error flags for libSRTP" OFF)
endif()

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/Modules)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(BUILD_SHARED_LIBS OFF) # to force usrsctp to be built static

if(WIN32)
	add_definitions(-DWIN32_LEAN_AND_MEAN)
	if(MSVC)
		add_definitions(-DNOMINMAX)
		add_definitions(-D_CRT_SECURE_NO_WARNINGS)
	endif()
endif()

add_definitions( -Wno-write-strings -Wno-missing-field-initializers -Wno-unused-parameter -Wno-unused-value -Wno-deprecated-declarations  -Wno-extra -Wno-switch -Wno-unused-but-set-variable)

set(LIBDATACHANNEL_SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/src/candidate.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/channel.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/configuration.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/datachannel.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/description.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/mediahandler.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/global.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/message.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/peerconnection.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/rtcpreceivingsession.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/track.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/rtppacketizationconfig.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/rtcpsrreporter.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/rtppacketizer.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/rtpdepacketizer.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/h264rtppacketizer.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/h264rtpdepacketizer.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/nalunit.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/h265rtppacketizer.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/h265nalunit.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/av1rtppacketizer.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/rtcpnackresponder.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/rtp.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/plihandler.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/pacinghandler.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/rembhandler.cpp
)


set(LIBSTUN_SOURCES
 ${CMAKE_CURRENT_SOURCE_DIR}/stun/src/Agent.cpp    
 ${CMAKE_CURRENT_SOURCE_DIR}/stun/src/Connection.cpp 
 ${CMAKE_CURRENT_SOURCE_DIR}/stun/src/Message.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/stun/src/sdpcommon.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/stun/src/Types.cpp 
 ${CMAKE_CURRENT_SOURCE_DIR}/stun/src/Utils.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/stun/src/Attribute.cpp  
 ${CMAKE_CURRENT_SOURCE_DIR}/stun/src/Reader.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/stun/src/TransportTuple.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/stun/src/utils.cpp 
 ${CMAKE_CURRENT_SOURCE_DIR}/stun/src/Writer.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/stun/src/Settings.cpp
 ${CMAKE_CURRENT_SOURCE_DIR}/stun/src/RestAPI.cpp

 #${CMAKE_CURRENT_SOURCE_DIR}/stun/src/tls.cpp 
 #${CMAKE_CURRENT_SOURCE_DIR}/stun/src/configuration.cpp
 #${CMAKE_CURRENT_SOURCE_DIR}/stun/src/icetransport.cpp 
 #${CMAKE_CURRENT_SOURCE_DIR}/stun/src/transport.cpp      
 #${CMAKE_CURRENT_SOURCE_DIR}/stun/src/candidate.cpp 

)



set(LIBDATACHANNEL_HEADERS
	${CMAKE_CURRENT_SOURCE_DIR}/include/rtc/candidate.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/include/rtc/channel.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/include/rtc/configuration.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/include/rtc/datachannel.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/include/rtc/description.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/include/rtc/mediahandler.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/include/rtc/rtcpreceivingsession.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/include/rtc/common.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/include/rtc/global.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/include/rtc/message.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/include/rtc/frameinfo.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/include/rtc/peerconnection.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/include/rtc/reliability.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/include/rtc/rtc.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/rtc/rtc.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/include/rtc/rtp.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/include/rtc/track.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/include/rtc/rtppacketizationconfig.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/include/rtc/rtcpsrreporter.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/include/rtc/rtppacketizer.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/include/rtc/rtpdepacketizer.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/include/rtc/h264rtppacketizer.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/include/rtc/h264rtpdepacketizer.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/include/rtc/nalunit.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/include/rtc/h265rtppacketizer.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/include/rtc/h265nalunit.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/include/rtc/av1rtppacketizer.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/include/rtc/rtcpnackresponder.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/include/rtc/utils.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/include/rtc/plihandler.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/include/rtc/pacinghandler.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/include/rtc/rembhandler.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/include/rtc/version.h
)

set(LIBDATACHANNEL_IMPL_SOURCES
	#${CMAKE_CURRENT_SOURCE_DIR}/src/impl/certificate.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/impl/channel.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/impl/datachannel.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/impl/dtlssrtptransport.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/impl/dtlstransport.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/impl/icetransport.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/impl/init.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/impl/peerconnection_imp.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/impl/logcounter.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/impl/sctptransport.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/impl/threadpool.cpp
	#${CMAKE_CURRENT_SOURCE_DIR}/src/impl/tls.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/impl/track.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/impl/utils.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/impl/processor.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/impl/sha.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/impl/pollinterrupter.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/impl/pollservice.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/impl/http.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/impl/httpproxytransport.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/impl/tcpserver.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/impl/tcptransport.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/impl/tlstransport.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/impl/transport.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/impl/verifiedtlstransport.cpp
#	${CMAKE_CURRENT_SOURCE_DIR}/src/impl/wstransport.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/impl/wshandshake.cpp
)

set(LIBDATACHANNEL_IMPL_HEADERS
	#${CMAKE_CURRENT_SOURCE_DIR}/src/impl/certificate.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/impl/channel.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/impl/datachannel.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/impl/dtlssrtptransport.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/impl/dtlstransport.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/impl/icetransport.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/impl/init.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/impl/internals.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/impl/peerconnection_imp.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/impl/queue.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/impl/logcounter.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/impl/sctptransport.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/impl/threadpool.hpp
#	${CMAKE_CURRENT_SOURCE_DIR}/src/impl/tls.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/impl/track.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/impl/utils.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/impl/processor.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/impl/sha.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/impl/pollinterrupter.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/impl/pollservice.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/impl/http.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/impl/httpproxytransport.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/impl/tcpserver.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/impl/tcptransport.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/impl/tlstransport.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/impl/transport.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/impl/verifiedtlstransport.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/impl/wstransport.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/impl/wshandshake.hpp
)

set(TESTS_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/test/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test/connectivity.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test/negotiated.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test/reliability.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test/turn_connectivity.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test/track.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test/capi_connectivity.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test/capi_track.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test/websocket.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test/websocketserver.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test/capi_websocketserver.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test/benchmark.cpp
)

set(TESTS_UWP_RESOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/test/uwp/tests/Logo.png
	${CMAKE_CURRENT_SOURCE_DIR}/test/uwp/tests/package.appxManifest
	${CMAKE_CURRENT_SOURCE_DIR}/test/uwp/tests/SmallLogo.png
	${CMAKE_CURRENT_SOURCE_DIR}/test/uwp/tests/SmallLogo44x44.png
	${CMAKE_CURRENT_SOURCE_DIR}/test/uwp/tests/SplashScreen.png
	${CMAKE_CURRENT_SOURCE_DIR}/test/uwp/tests/StoreLogo.png
	${CMAKE_CURRENT_SOURCE_DIR}/test/uwp/tests/Windows_TemporaryKey.pfx
)

set(BENCHMARK_UWP_RESOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/test/uwp/benchmark/Logo.png
	${CMAKE_CURRENT_SOURCE_DIR}/test/uwp/benchmark/package.appxManifest
	${CMAKE_CURRENT_SOURCE_DIR}/test/uwp/benchmark/SmallLogo.png
	${CMAKE_CURRENT_SOURCE_DIR}/test/uwp/benchmark/SmallLogo44x44.png
	${CMAKE_CURRENT_SOURCE_DIR}/test/uwp/benchmark/SplashScreen.png
	${CMAKE_CURRENT_SOURCE_DIR}/test/uwp/benchmark/StoreLogo.png
	${CMAKE_CURRENT_SOURCE_DIR}/test/uwp/benchmark/Windows_TemporaryKey.pfx
)

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)


if(SCTP_DEBUG)
	#add_definitions(-DSCTP_DEBUG)
endif()

if(USE_SYSTEM_USRSCTP)
	find_package(Usrsctp REQUIRED)
else()
	option(sctp_build_shared_lib OFF)
	option(sctp_build_programs OFF)
	option(sctp_inet OFF)
	option(sctp_inet6 OFF)
	option(sctp_werror OFF)
	set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
	add_subdirectory(deps/usrsctp EXCLUDE_FROM_ALL)
	if (MSYS OR MINGW)
		target_compile_definitions(usrsctp PUBLIC -DSCTP_STDINT_INCLUDE=<stdint.h>)
	endif()
	add_library(Usrsctp::Usrsctp ALIAS usrsctp)

	if(INSTALL_DEPS_LIBS)
		install(TARGETS usrsctp EXPORT LibDataChannelTargets)
		# Fix directories
		set_target_properties(usrsctp PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "")
		target_include_directories(usrsctp INTERFACE
			$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/deps/usrsctp/usrsctplib>
			$<INSTALL_INTERFACE:>)
	endif()
endif()

if(RTC_UPDATE_VERSION_HEADER)
	configure_file (
		${PROJECT_SOURCE_DIR}/cmake/version.h.in
		${CMAKE_CURRENT_SOURCE_DIR}/include/rtc/version.h
	)
endif()







add_subdirectory(deps/libsrtp)

if(USE_MBEDTLS)  
file(GLOB SOURCES  "../libuv/src/*.cpp" "../libuv/src/unix/*.cpp" "../src/*.cpp" "../src/x64/*.cpp" "../base/src/*.cpp"   "../http/src/*.cpp" "../http_parser/*.cpp" "../facedetec/src/configuration.cpp" "../signal/src/*.cpp" "../net/src/*.cpp" "../net/srcmbedtls/*.cpp"  )
else()
file(GLOB SOURCES  "../libuv/src/*.cpp" "../libuv/src/unix/*.cpp" "../src/*.cpp" "../src/x64/*.cpp" "../base/src/*.cpp"  "../net/src/*.cpp" "../http/src/*.cpp" "../http_parser/*.cpp" "../facedetec/src/configuration.cpp" "../signal/src/*.cpp" )
endif()   







add_library(datachannel-static STATIC EXCLUDE_FROM_ALL
	${LIBDATACHANNEL_SOURCES}
	${LIBSTUN_SOURCES}
	${LIBDATACHANNEL_HEADERS}
	${LIBDATACHANNEL_IMPL_SOURCES}
        ${SOURCES}
	${LIBDATACHANNEL_IMPL_HEADERS})
set_target_properties(datachannel-static PROPERTIES
	VERSION ${PROJECT_VERSION}
	CXX_STANDARD 17)
target_compile_definitions(datachannel-static PRIVATE RTC_EXPORTS)
target_compile_definitions(datachannel-static PUBLIC RTC_STATIC)



target_include_directories(datachannel-static PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)
target_include_directories(datachannel-static PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/rtc)
target_include_directories(datachannel-static PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_include_directories(datachannel-static PUBLIC 
    stun/include
    deps/libsrtp/config
    deps/libsrtp/crypto/include
    deps/libsrtp/include
    deps/json/include
    deps/usrsctp/usrsctplib
    ${INC} ../signal/include ../facedetec/include/ ${SSL_ROOT}/include  ../include  ../http/include/ ../base/include/  ../libuv/include ../libuv/src ../build/ ../http_parser/ ../net/dtls/ ../net/include/
)



if(USE_MBEDTLS) 
target_include_directories(datachannel-static PUBLIC 
../net/incmbedtls/)
endif()   


target_link_libraries(datachannel-static PRIVATE Threads::Threads)
target_link_libraries(datachannel-static PRIVATE Usrsctp::Usrsctp )



if(WIN32)
	target_link_libraries(datachannel-static PUBLIC ws2_32) # winsock2
endif()

if (NO_WEBSOCKET)
	target_compile_definitions(datachannel-static PUBLIC RTC_ENABLE_WEBSOCKET=0)
else()
	target_compile_definitions(datachannel-static PUBLIC RTC_ENABLE_WEBSOCKET=1)
endif()

if(NO_MEDIA)
	target_compile_definitions(datachannel-static PUBLIC RTC_ENABLE_MEDIA=0)
else()
	target_compile_definitions(datachannel-static PUBLIC RTC_ENABLE_MEDIA=1)
	if(USE_SYSTEM_SRTP)
		target_compile_definitions(datachannel-static PRIVATE RTC_SYSTEM_SRTP=1)
		target_link_libraries(datachannel-static PRIVATE srtp2)
	else()
		if(NOT TARGET srtp2)
			#add_subdirectory(deps/libsrtp)
		endif()
		target_compile_definitions(datachannel-static PRIVATE RTC_SYSTEM_SRTP=0)
		target_link_libraries(datachannel-static PRIVATE srtp2)
	endif()
endif()


if(USE_MBEDTLS)
	#if(NOT TARGET MbedTLS::MbedTLS)
	#    set(MbedTLS_DEBUG 1)
	#	find_package(MbedTLS REQUIRED)
	#endif()

 
 	option(ENABLE_TESTING OFF)
	option(ENABLE_PROGRAMS OFF)
	set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)

	add_subdirectory(deps/mbedtls EXCLUDE_FROM_ALL)

	target_compile_definitions(datachannel-static PRIVATE USE_MBEDTLS=1)

	target_include_directories(datachannel-static PRIVATE ${CMAKE_SOURCE_DIR}/deps/mbedtls/include)
	target_link_libraries(datachannel-static  PRIVATE mbedtls mbedx509 mbedcrypto -lz)
	
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
	target_compile_definitions(datachannel-static PRIVATE USE_GNUTLS=0)
	#target_link_libraries(datachannel PRIVATE ${SSL_LIB}/libssl.a ${SSL_LIB}/libcrypto.a)
	target_link_libraries(datachannel-static PRIVATE ${SSL_LIB}/libssl.a ${SSL_LIB}/libcrypto.a -lz)
endif()


target_compile_definitions(datachannel-static PRIVATE USE_NICE=0)


if(CAPI_STDCALL)
	target_compile_definitions(datachannel-static PUBLIC CAPI_STDCALL)
endif()



set_target_properties(datachannel-static PROPERTIES EXPORT_NAME LibDataChannelStatic)
add_library(LibDataChannel::LibDataChannelStatic ALIAS datachannel-static)

if(NOT MSVC)
	target_compile_options(datachannel-static PRIVATE -Wall -Wextra)
endif()

if(WARNINGS_AS_ERRORS)
	if(MSVC)
		target_compile_options(datachannel-static PRIVATE /WX)
	else()
		target_compile_options(datachannel-static PRIVATE -Werror)
	endif()
endif()


#add_compile_options(-fsanitize=address)
#add_link_options(-fsanitize=address)




# Examples
if(NOT NO_EXAMPLES)

	if(NOT NO_MEDIA AND NOT NO_WEBSOCKET)
    	add_subdirectory(examples/streamer)
	endif()

endif()

