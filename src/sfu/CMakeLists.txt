
cmake_minimum_required(VERSION 3.7)

if (${ANDROID_ABI} STREQUAL "x86_64")

SET(SSL_ROOT "/workspace/MagicAI/src/openssl/buildx64")
SET(SSL_LIB "/workspace/MagicAI/src/openssl/buildx64/lib")

SET(WEBRTC_REPO "${CMAKE_SOURCE_DIR}/webrtc/src")


else(${ANDROID_ABI} STREQUAL "mipsel")
 
SET(SSL_ROOT "/workspace/MagicAI/src/openssl/buildt31")
SET(SSL_LIB "/workspace/MagicAI/src/openssl/buildt31/lib")
SET(T31SDK "/workspace/adappt/T31/ISVP-T31-1.1.6-20221229/software/Ingenic-SDK-T31-1.1.6-20221229/sdk/5.4.0")

SET(WEBRTC_REPO "${CMAKE_SOURCE_DIR}/webrtc/src")


endif()




#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -g")
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set(CMAKE_POSITION_INDEPENDENT_CODE ON)






if (${ANDROID_ABI} STREQUAL "x86_64")

file(GLOB SOURCES "sfuserver/*.cpp" "src/*.cpp" "src/RTCP/*.cpp" "src/Codecs/*.cpp" "src/sdp/*.cpp" "src/SctpDictionaries/*.cpp" "src/Channel/*.cpp" "src/Utils/*.cpp" "src/RtpDictionaries/*.cpp" "../libuv/src/*.cpp" "../libuv/src/unix/*.cpp" "../base/src/*.cpp" "net/src/*.cpp"  "http/src/*.cpp" "../http_parser/*.cpp" "../json/src/*.cpp" "../signal/src/*.cpp" )

else(${ANDROID_ABI} STREQUAL "mipsel")
 
file(GLOB SOURCES "*.cpp" "../../libuv/src/*.cpp" "../../libuv/src/unix/*.cpp" "../src/*.cpp" "../src/t31/*.cpp" "../../base/src/*.cpp"  "../../net/src/*.cpp" "../../http/src/*.cpp" "../../http_parser/*.cpp" "../../facedetec/src/*.cpp" "../../facedetec/src/t31/*.cpp" "../../signal/src/*.cpp")

endif()





#set (CMAKE_POSITION_INDEPENDENT_CODE OFF)
#set (CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -fno-pie")
#set (CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} -fno-pie")
#set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-no-pie")



add_executable(runWebrtc  ${SOURCES})




#target_compile_definitions(runWebrtc PRIVATE HTTPSSL  WEBRTC_POSIX NDEBUG )


set(INC "/workspace/MagicAI/src/qrcode/include")


if (${ANDROID_ABI} STREQUAL "x86_64")

target_include_directories(runWebrtc PRIVATE   ${INC} include/ ../json/include/ ../json/include/json ../signal/include  http/include/ ../base/include/ net/include/ ../libuv/include ../libuv/src  ../http_parser/ deps/openssl/openssl/include deps/usrsctp/usrsctp/usrsctplib deps/libsdptransform/include/ deps/libsrtp/srtp/include deps/libwebrtc/ deps/libwebrtc/libwebrtc deps/libwebrtc/deps/abseil-cpp/abseil-cpp   )

else(${ANDROID_ABI} STREQUAL "mipsel")
 
target_include_directories(runWebrtc PRIVATE  ${INC} ../../signal/include ../../facedetec/include/ ../../facedetec/include/t31/  ${SSL_ROOT}/include  ../include  ../../http/include/ ../../base/include/ ../../net/include/ ../../libuv/include ../../libuv/src ../../../build/ ../../http_parser/ ../webrtc/include   ../include/x64    ../include/t31 ../../facedetec/xailient-sdk-HD/xailient-sdk-2.1.0.2024/include/ ${T31SDK}/include ${T31SDK}/samples/libimp-sample/ ${JZDL_SDK_FOLDER_INC})

endif()




#target_compile_options(runWebrtc PRIVATE -Wno-return-type -Wno-format-overflow -Wno-format -Wno-unused-but-set-variable -Wno-write-strings -no-pie)
target_compile_features(runWebrtc PRIVATE cxx_std_14)

target_link_directories(runWebrtc PRIVATE  ../../facedetec/xailient-sdk-HD/xailient-sdk-2.1.0.2024/lib  ${T31SDK}/lib ${INSTALLPATH}/mips-linux-gnu/lib  /workspace/magik-toolkit/InferenceKit/jzdl/5.4.0/lib/uclibc ${JZDL_SDK_FOLDER_LIB})


#add_subdirectory( webrtc/third_party)
add_subdirectory(deps/libsdptransform)
add_subdirectory(deps/usrsctp)


SET(SAT_LIB "/workspace/MagicAI/src/sfu/out/Release")


if (${ANDROID_ABI} STREQUAL "x86_64")

target_link_libraries(runWebrtc PRIVATE  sdptransform usrsctp ${SAT_LIB}/libsrtp.a ${SAT_LIB}/libwebrtc.a ${SAT_LIB}/libabseil.a   pthread m dl   ${SSL_LIB}/libssl.a ${SSL_LIB}/libcrypto.a  )

else(${ANDROID_ABI} STREQUAL "mipsel")
 
target_link_libraries(runWebrtc PRIVATE  
    srtp   usrsctp  event  absl::optional  absl::variant   absl::strings 
      rt m dl -pthread   ${SSL_LIB}/libssl.so ${SSL_LIB}/libcrypto.so  ${T31SDK}/lib/uclibc/libalog.a ${T31SDK}/lib/uclibc/libimp.a  /workspace/MagicAI/src/qrcode/rgbQr/zbar.a xailientsdk xailientsdkutils jzdl.m )

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


# add at first, avoid header search path, definition chaos






if (${ANDROID_ABI} STREQUAL "arm64-v8a")
    target_compile_definitions(runWebrtc PRIVATE -DWEBRTC_ARCH_ARM64 -DWEBRTC_HAS_NEON)
elseif (${ANDROID_ABI} STREQUAL "armeabi-v7a")
    target_compile_definitions(runWebrtc PRIVATE -DWEBRTC_ARCH_ARM -DWEBRTC_ARCH_ARM_V7 -DWEBRTC_HAS_NEON )
endif()



target_compile_definitions(runWebrtc PRIVATE -DHAVE_NETINET_IN_H   -DNDEBUG -DMS_LITTLE_ENDIAN -D_GNU_SOURCE -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -DDEBUG -DMS_LOG_TRACE -DMS_LOG_FILE_LINE
)

target_include_directories(runWebrtc PRIVATE  

    
) 


if (${ANDROID_ABI} STREQUAL "x86_64")

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g")

else(${ANDROID_ABI} STREQUAL "mipsel")
 
set(CMAKE_BUILD_TYPE MinSizeRel)

endif()


if (CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
    target_compile_options(runWebrtc PRIVATE -Os -flto -s -Wl,-s -Wl,--gc-sections -fpermissive -DNDEBUG -DMIPS32=1)
else()
target_compile_options(runWebrtc PRIVATE -fpermissive -DNDEBUG)
endif()

