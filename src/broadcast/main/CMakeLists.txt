
cmake_minimum_required(VERSION 3.7)

set(WEBRTC_LIBS "/workspace/MagicAI/src/webrtc/build/")
SET(WEBRTC_PATH "/workspace/webrtc/src/")

if (${ANDROID_ABI} STREQUAL "x86_64")

SET(SSL_ROOT "/workspace/MagicAI/src/openssl/buildx64")
SET(SSL_LIB "/workspace/MagicAI/src/openssl/buildx64/lib")

else(${ANDROID_ABI} STREQUAL "mipsel")
 
SET(SSL_ROOT "/workspace/MagicAI/src/openssl/buildt31")
SET(SSL_LIB "/workspace/MagicAI/src/openssl/buildt31/lib")
SET(T31SDK "/workspace/adappt/T31/ISVP-T31-1.1.6-20221229/software/Ingenic-SDK-T31-1.1.6-20221229/sdk/5.4.0")

endif()



#cmake_minimum_required(VERSION 3.4.1)
#SET(WEBRTC_PATH_INC "/workspace/webrtc_android/src")
#SET(WEBRTC_PATH_LIB "/workspace/webrtc_android/src/out/m75/obj")
#SET(FFMPEG_ROOT "/workspace/ffmpeg")
#SET(NVR "/workspace/nvrmain/acvs-videoedge-nvr/NVR")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g")
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set(CMAKE_POSITION_INDEPENDENT_CODE ON)




if (${ANDROID_ABI} STREQUAL "x86_64")

file(GLOB SOURCES "*.cpp" "../../libuv/src/*.cpp" "../../libuv/src/unix/*.cpp" "../src/*.cpp" "../src/x64/*.cpp" "../../base/src/*.cpp"  "../../net/src/*.cpp" "../../http/src/*.cpp" "../../http_parser/*.cpp" "../../facedetec/src/configuration.cpp" "../../signal/src/*.cpp" )

else(${ANDROID_ABI} STREQUAL "mipsel")
 
file(GLOB SOURCES "*.cpp" "../../libuv/src/*.cpp" "../../libuv/src/unix/*.cpp" "../src/*.cpp" "../src/t31/*.cpp" "../../base/src/*.cpp"  "../../net/src/*.cpp" "../../http/src/*.cpp" "../../http_parser/*.cpp" "../../facedetec/src/*.cpp" "../../facedetec/src/t31/*.cpp" "../../signal/src/*.cpp")

endif()








#set (CMAKE_POSITION_INDEPENDENT_CODE OFF)
#set (CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -fno-pie")
#set (CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} -fno-pie")
#set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-no-pie")



add_executable(runWebrtc  ${SOURCES})

target_compile_definitions(runWebrtc PRIVATE HTTPSSL  WEBRTC_POSIX NDEBUG )


set(INC "/workspace/MagicAI/src/qrcode/include")


if (${ANDROID_ABI} STREQUAL "x86_64")

target_include_directories(runWebrtc PRIVATE  ${INC}  ../../signal/include ../../facedetec/include/ ${SSL_ROOT}/include  ../include  ../../http/include/ ../../base/include/ ../../net/include/ ../../libuv/include ../../libuv/src ../../../build/ ../../http_parser/ ../webrtc/include   ../include/x64 ${WEBRTC_PATH}    ${WEBRTC_PATH}/third_party/abseil-cpp  )

else(${ANDROID_ABI} STREQUAL "mipsel")
 
target_include_directories(runWebrtc PRIVATE  ${INC} ../../signal/include ../../facedetec/include/ ../../facedetec/include/t31/  ${SSL_ROOT}/include  ../include  ../../http/include/ ../../base/include/ ../../net/include/ ../../libuv/include ../../libuv/src ../../../build/ ../../http_parser/ ../webrtc/include   ../include/x64 ${WEBRTC_PATH}    ${WEBRTC_PATH}/third_party/abseil-cpp   ../include/t31 ${WEBRTC_PATH}/third_party/abseil-cpp ../../facedetec/xailient-sdk/include/ ${T31SDK}/include ${T31SDK}/samples/libimp-sample/)

endif()





#target_compile_options(runWebrtc PRIVATE -Wno-return-type -Wno-format-overflow -Wno-format -Wno-unused-but-set-variable -Wno-write-strings -no-pie)
target_compile_features(runWebrtc PRIVATE cxx_std_11)

target_link_directories(runWebrtc PRIVATE  ../../facedetec/xailient-sdk/lib  ${T31SDK}/lib ${INSTALLPATH}/mips-linux-gnu/lib)






if (${ANDROID_ABI} STREQUAL "x86_64")

target_link_libraries(runWebrtc PRIVATE jingle_peerconnection_so  rnnoise
    srtp   usrsctp  event    absl::optional    absl::variant    absl::strings
    pffft pthread m dl z  ${SSL_LIB}/libssl.a ${SSL_LIB}/libcrypto.a  /workspace/MagicAI/src/qrcode/rgbQr/zbar.a)

else(${ANDROID_ABI} STREQUAL "mipsel")
 
target_link_libraries(runWebrtc PRIVATE jingle_peerconnection_so  rnnoise
    srtp   usrsctp  event  absl::optional  absl::variant   absl::strings 
    pffft xailient-fi  rt m dl -pthread atomic  ${SSL_LIB}/libssl.a ${SSL_LIB}/libcrypto.a  ${T31SDK}/lib/uclibc/libalog.a ${T31SDK}/lib/uclibc/libimp.a /workspace/MagicAI/src/qrcode/rgbQr/zbar.a)

endif()




#${WEBRTC_LIBS}/third_party/abseil-cpp/abseil-cpp/absl/strings/libabsl_absl_strings.a #${WEBRTC_LIBS}/third_party/abseil-cpp/abseil-cpp/absl/types/libabsl_absl_bad_variant_access.a #${WEBRTC_LIBS}/third_party/abseil-cpp/abseil-cpp/absl/types/libabsl_absl_bad_optional_access.a  


add_subdirectory(webrtc)
#add_subdirectory(libuv)

set(CMAKE_BUILD_TYPE MinSizeRel)
#set(CMAKE_BUILD_TYPE Debug)
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g")
#set(CMAKE_C_COMPILER "gcc")
#set(CMAKE_CXX_COMPILER "g++")


if (CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
    target_compile_options(runWebrtc PRIVATE -Os -flto -s -Wl,-s -Wl,--gc-sections -fpermissive -DNDEBUG)
else()
target_compile_options(runWebrtc PRIVATE -fpermissive -DNDEBUG)
endif()








