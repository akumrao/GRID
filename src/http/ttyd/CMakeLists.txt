cmake_minimum_required(VERSION 3.12.0)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

project(ttyd VERSION 1.7.7 LANGUAGES CXX)


set(TTYD_VERSION "${PROJECT_VERSION}")

set(CMAKE_CXX_STANDARD 17)

# Enforce the standard; if the compiler is too old, the build fails
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Options
option(USE_MBEDTLS "Use Mbed TLS instead of OpenSSL" OFF)



message("Current Source Directory: ${CMAKE_CURRENT_SOURCE_DIR}/../openssl/buildx64")
message("Current Binary Directory: ${CMAKE_CURRENT_BINARY_DIR}")
message("Current List File Directory: ${CMAKE_CURRENT_LIST_DIR}/../openssl/buildx64")


SET(SSL_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../openssl/buildx64")
SET(SSL_LIB "${CMAKE_CURRENT_LIST_DIR}/../../openssl/buildx64/lib")




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
		add_compile_options(/W2)
	endif()

else()
add_definitions( -Wno-write-strings -Wno-missing-field-initializers -Wno-unused-parameter -Wno-unused-value -Wno-deprecated-declarations  -Wno-extra -Wno-switch -Wno-unused-but-set-variable)
endif()





set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

set(SOURCE_FILES httptests.cpp)


set(SOURCE_FILES src/utils.cpp src/pty.cpp src/protocol.cpp src/server.cpp)

file(GLOB SOURCES   
    ../src/*.cpp
    ../../base/src/*.cpp
    ../../net/src/*.cpp
#    ../../net/uv_multiplex/src/*.cpp
    ../../http_parser/*.cpp)


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


add_executable(${PROJECT_NAME} ${SOURCE_FILES} ${SOURCES})
target_include_directories(${PROJECT_NAME} PUBLIC ${INCLUDE_DIRS})
#target_link_libraries(${PROJECT_NAME}  uv_a)
target_compile_definitions(${PROJECT_NAME} PUBLIC
    TTYD_VERSION="${TTYD_VERSION}"
    $<$<PLATFORM_ID:Windows>:_WIN32_WINNT=0xa00 WINVER=0xa00>
)


target_include_directories(${PROJECT_NAME} PUBLIC 
          ${SSL_ROOT}/include  ../include/ ../../net/include/ ../../base/include ../../http_parser/ ../../json/include
  ../../libuv/include ../../libuv/src)



if(USE_MBEDTLS) 
target_include_directories(${PROJECT_NAME} PUBLIC 
../net/incmbedtls/)
endif()   


#target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads)



if(WIN32)
	target_link_libraries(${PROJECT_NAME} PUBLIC ws2_32) # winsock2
endif()



if(USE_MBEDTLS)
	#if(NOT TARGET MbedTLS::MbedTLS)
	#    set(MbedTLS_DEBUG 1)
	#	find_package(MbedTLS REQUIRED)
	#endif()

 
 	option(ENABLE_TESTING OFF)
	option(ENABLE_PROGRAMS OFF)
	set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)

	add_subdirectory(${CMAKE_SOURCE_DIR}/../../webrtcserver/deps/mbedtls build2 EXCLUDE_FROM_ALL)

	target_compile_definitions(${PROJECT_NAME} PRIVATE USE_MBEDTLS=1 HTTPSSL=1)

	target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/../../webrtcserver/deps/mbedtls/include)
	target_link_libraries(${PROJECT_NAME}  PRIVATE  ${LINK_LIBS} uv_a mbedtls mbedx509 mbedcrypto -lz)
	
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
	target_compile_definitions(${PROJECT_NAME} PRIVATE HTTPSSL=1)
	#target_link_libraries(${PROJECT_NAME} ${SSL_LIB}/libssl.a ${SSL_LIB}/libcrypto.a)
	target_link_libraries(${PROJECT_NAME} PRIVATE  ${LINK_LIBS} uv_a ${SSL_LIB}/libssl.a ${SSL_LIB}/libcrypto.a -lz)
endif()




if(CAPI_STDCALL)
	target_compile_definitions(${PROJECT_NAME} PUBLIC CAPI_STDCALL)
endif()


if(UNIX)
target_compile_options(${PROJECT_NAME} PRIVATE -fpermissive -Wno-return-type -Wno-format-overflow -Wno-format -Wno-unused-but-set-variable -Wno-write-strings -no-pie)
endif()

if(WIN32)
target_compile_options(${PROJECT_NAME} PRIVATE )
endif()



if(NOT MSVC)
	target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra)
endif()

if(WARNINGS_AS_ERRORS)
	if(MSVC)
		target_compile_options(${PROJECT_NAME} PRIVATE /WX)
	else()
		target_compile_options(${PROJECT_NAME} PRIVATE -Werror)
	endif()
endif()



