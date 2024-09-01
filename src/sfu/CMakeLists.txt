cmake_minimum_required(VERSION 3.7)

if (${CROSS_PLATEFORM} STREQUAL "x86_64")

SET(SSL_ROOT "/workspace/MagicAI/src/openssl/buildx64")
SET(SSL_LIB "/workspace/MagicAI/src/openssl/buildx64/lib")


elseif (${CROSS_PLATEFORM} STREQUAL "mipsel")
 
SET(SSL_ROOT "/workspace/MagicAI/src/openssl/buildt31")
SET(SSL_LIB "/workspace/MagicAI/src/openssl/buildt31/lib")
SET(T31SDK "/workspace/adappt/T31/ISVP-T31-1.1.6-20221229/software/Ingenic-SDK-T31-1.1.6-20221229/sdk/5.4.0")

else(${CROSS_PLATEFORM} STREQUAL "android")

SET(SSL_ROOT "/workspace/MagicAI/src/sfu/deps/openssl/build/openssl/arm64-v8a/")
SET(SSL_LIB "/workspace/MagicAI/src/sfu/deps/openssl/build/openssl/arm64-v8a/lib")

endif()


#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -g")
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set(CMAKE_POSITION_INDEPENDENT_CODE ON)



if (${CROSS_PLATEFORM} STREQUAL "x86_64")

file(GLOB SOURCES "sfuserver/*.cpp" "src/*.cpp" "src/RTCP/*.cpp" "src/Codecs/*.cpp" "src/sdp/*.cpp" "src/SctpDictionaries/*.cpp" "src/Channel/*.cpp" "src/Utils/*.cpp" "src/RtpDictionaries/*.cpp" "../libuv/src/*.cpp" "../libuv/src/unix/*.cpp" "../base/src/*.cpp" "net/src/*.cpp"  "http/src/*.cpp" "../http_parser/*.cpp" "../json/src/*.cpp" "../signal/src/*.cpp" )

add_executable(runWebrtc  ${SOURCES})

elseif(${CROSS_PLATEFORM} STREQUAL "mipsel")
 
file(GLOB SOURCES "sfuserver/*.cpp" "src/*.cpp" "src/RTCP/*.cpp" "src/Codecs/*.cpp" "src/sdp/*.cpp" "src/SctpDictionaries/*.cpp" "src/Channel/*.cpp" "src/Utils/*.cpp" "src/RtpDictionaries/*.cpp" "../libuv/src/*.cpp" "../libuv/src/unix/*.cpp" "../base/src/*.cpp" "net/src/*.cpp"  "http/src/*.cpp" "../http_parser/*.cpp" "../json/src/*.cpp" "../signal/src/*.cpp" )

add_executable(runWebrtc  ${SOURCES})

else(${CROSS_PLATEFORM} STREQUAL "android")
file(GLOB SOURCES "sfuserver/*.cpp" "src/*.cpp" "src/RTCP/*.cpp" "src/Codecs/*.cpp" "src/sdp/*.cpp" "src/SctpDictionaries/*.cpp" "src/Channel/*.cpp" "src/Utils/*.cpp" "src/RtpDictionaries/*.cpp" "../libuv/src/*.cpp" "../libuv/src/unix/*.cpp" "../base/src/*.cpp" "net/src/*.cpp"  "http/src/*.cpp" "../http_parser/*.cpp" "../json/src/*.cpp" "../signal/src/*.cpp" )

add_library(runWebrtc SHARED ${SOURCES})

endif()





#set (CMAKE_POSITION_INDEPENDENT_CODE OFF)
#set (CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -fno-pie")
#set (CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} -fno-pie")
#set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-no-pie")







#target_compile_definitions(runWebrtc PRIVATE HTTPSSL  WEBRTC_POSIX NDEBUG )


set(INC "/workspace/MagicAI/src/qrcode/include")


if (${CROSS_PLATEFORM} STREQUAL "x86_64")

target_include_directories(runWebrtc PRIVATE   ${INC} ${SSL_ROOT}/include include/ ../json/include/ ../json/include/json ../signal/include  http/include/ ../base/include/ net/include/ ../libuv/include ../libuv/src  ../http_parser/ deps/openssl/openssl/include deps/usrsctp/usrsctp/usrsctplib deps/libsdptransform/include/ deps/libsrtp/srtp/include deps/libwebrtc/ deps/libwebrtc/libwebrtc deps/libwebrtc/abseil-cpp   )

elseif(${CROSS_PLATEFORM} STREQUAL "mipsel")
 
target_include_directories(runWebrtc PRIVATE   ${INC} ${SSL_ROOT}/include include/ ../json/include/ ../json/include/json ../signal/include  http/include/ ../base/include/ net/include/ ../libuv/include ../libuv/src  ../http_parser/ deps/openssl/openssl/include deps/usrsctp/usrsctp/usrsctplib deps/libsdptransform/include/ deps/libsrtp/srtp/include deps/libwebrtc/ deps/libwebrtc/libwebrtc deps/libwebrtc/abseil-cpp   )

else(${CROSS_PLATEFORM} STREQUAL "android")

target_include_directories(runWebrtc PRIVATE   ${INC} ${SSL_ROOT}/include include/ ../json/include/ ../json/include/json ../signal/include  http/include/ ../base/include/ net/include/ ../libuv/include ../libuv/src  ../http_parser/ deps/openssl/openssl/include deps/usrsctp/usrsctp/usrsctplib deps/libsdptransform/include/ deps/libsrtp/srtp/include deps/libwebrtc/ deps/libwebrtc/libwebrtc deps/libwebrtc/abseil-cpp   )

endif()




#target_compile_options(runWebrtc PRIVATE -Wno-return-type -Wno-format-overflow -Wno-format -Wno-unused-but-set-variable -Wno-write-strings -no-pie)

target_compile_features(runWebrtc PRIVATE cxx_std_14)



#add_subdirectory( webrtc/third_party)
add_subdirectory(deps/libsdptransform)
add_subdirectory(deps/usrsctp)
add_subdirectory(deps/libsrtp)
add_subdirectory(deps/libwebrtc)


if (${CROSS_PLATEFORM} STREQUAL "x86_64")

target_link_libraries(runWebrtc PRIVATE  sdptransform usrsctp srtp webrtc  absl::optional    absl::variant    absl::strings pthread m dl   ${SSL_LIB}/libssl.a ${SSL_LIB}/libcrypto.a )

elseif(${CROSS_PLATEFORM} STREQUAL "mipsel")
 
target_link_libraries(runWebrtc PRIVATE  sdptransform usrsctp srtp webrtc  absl::optional    absl::variant    absl::strings pthread m dl   ${SSL_LIB}/libssl.a ${SSL_LIB}/libcrypto.a )

else(${CROSS_PLATEFORM} STREQUAL "android")

target_link_libraries(runWebrtc PRIVATE  log sdptransform usrsctp srtp webrtc  absl::optional    absl::variant   absl::strings    ${SSL_LIB}/libssl.a ${SSL_LIB}/libcrypto.a )

endif()





set(CMAKE_C_COMPILER_WORKS 1)

set(CMAKE_CXX_COMPILER_WORKS 1)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Don't use e.g. GNU extension (like -std=gnu++11) for portability
set(CMAKE_CXX_EXTENSIONS OFF)

#set(CMAKE_POSITION_INDEPENDENT_CODE ON)


set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNDEBUG"  )

set(CMAKE_THREAD_PREFER_PTHREAD ON)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)



#if (${ANDROID_ABI} STREQUAL "arm64-v8a")
#    target_compile_definitions(runWebrtc PRIVATE -DWEBRTC_ARCH_ARM64 -DWEBRTC_HAS_NEON)
#elseif (${ANDROID_ABI} STREQUAL "armeabi-v7a")
#    target_compile_definitions(runWebrtc PRIVATE -DWEBRTC_ARCH_ARM -DWEBRTC_ARCH_ARM_V7 -DWEBRTC_HAS_NEON )
#endif()



target_compile_definitions(runWebrtc PRIVATE -DHAVE_NETINET_IN_H   -DNDEBUG -DMS_LITTLE_ENDIAN -D_GNU_SOURCE -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -DDEBUG -DMS_LOG_TRACE -DMS_LOG_FILE_LINE
)



if (${CROSS_PLATEFORM} STREQUAL "x86_64")

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g")

elseif (${CROSS_PLATEFORM} STREQUAL "mipsel")

set(CMAKE_BUILD_TYPE MinSizeRel)

else(${CROSS_PLATEFORM} STREQUAL "android")

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g")

endif()



#-DMS_BIG_ENDIAN  -D_GNU_SOURCE -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -DDEBUG -DMS_LOG_TRACE -DMS_LOG_FILE_LINE)

if (CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
    target_compile_options(runWebrtc PRIVATE -Os -flto -s -Wl,-s -Wl,--gc-sections -fpermissive -DNDEBUG -DMIPS32=1)
else()
target_compile_options(runWebrtc PRIVATE -fpermissive -DNDEBUG)
endif()







     
