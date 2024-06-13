
cmake_minimum_required(VERSION 3.7)

set(WEBRTC_LIBS "/workspace/MagicAI/src/webrtc/build/")
SET(WEBRTC_PATH "/workspace/webrtc/src/")
SET(SSL_ROOT "/workspace/MagicAI/src/openssl/buildx64")
SET(SSL_LIB "/workspace/MagicAI/src/openssl/buildx64/lib")


#cmake_minimum_required(VERSION 3.4.1)
#SET(WEBRTC_PATH_INC "/workspace/webrtc_android/src")
#SET(WEBRTC_PATH_LIB "/workspace/webrtc_android/src/out/m75/obj")
#SET(FFMPEG_ROOT "/workspace/ffmpeg")
#SET(NVR "/workspace/nvrmain/acvs-videoedge-nvr/NVR")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g")
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)



SET(THIRDPARTY "${NVR}/3rdParty")
SET(RTC "${NVR}/3rdParty/rtc/src")
SET(xstore "${NVR}/xStore")






file(GLOB SOURCES "*.cpp" "../src/*.cpp" "../src/x64/*.cpp" "../../libuv/src/*.cpp" "../../libuv/src/unix/*.cpp" "../../base/src/*.cpp"  "../../net/src/*.cpp" "../../http/src/*.cpp" "../../http_parser/*.cpp" "../../facedetec/src/configuration.cpp" "../../signal/src/*.cpp" )



#set(CMAKE_C_COMPILER "gcc")
#set(CMAKE_CXX_COMPILER "g++")

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g")





#set (CMAKE_POSITION_INDEPENDENT_CODE OFF)
#set (CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -fno-pie")
#set (CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} -fno-pie")
#set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-no-pie")



add_executable(runWebrtc  ${SOURCES})

target_compile_definitions(runWebrtc PRIVATE HTTPSSL  WEBRTC_POSIX  )


target_include_directories(runWebrtc PRIVATE   ../../signal/include ../../facedetec/include/ ${SSL_ROOT}/include  ../include  ../../http/include/ ../../base/include/ ../../net/include/ ../../libuv/include ../../libuv/src ../../../build/ ../../http_parser/ ../webrtc/include   ../include/x64 ${WEBRTC_PATH}    ${WEBRTC_PATH}/third_party/abseil-cpp  )


#target_link_options(runWebrtc PRIVATE -no-pie)

#target_compile_options(runWebrtc PRIVATE -Wno-return-type -Wno-format-overflow -Wno-format -Wno-unused-but-set-variable -Wno-write-strings -no-pie)
target_compile_features(runWebrtc PRIVATE cxx_std_11)

#target_link_directories(runWebrtc PRIVATE ${xstore}/appliance/build/root/usr/lib64/ ${NVR}/deliverables/staging/rootfs/opt/americandynamics/venvr/lib/ /opt/intel/mediasdk/lib64 /opt/intel/mediasdk/lib/lin_x64)

target_link_libraries(runWebrtc PRIVATE jingle_peerconnection_so  rnnoise
    srtp   usrsctp  event    absl::optional    absl::variant    absl::strings
    pffft pthread m dl z  ${SSL_LIB}/libssl.a ${SSL_LIB}/libcrypto.a  )


#${WEBRTC_LIBS}/third_party/abseil-cpp/abseil-cpp/absl/strings/libabsl_absl_strings.a #${WEBRTC_LIBS}/third_party/abseil-cpp/abseil-cpp/absl/types/libabsl_absl_bad_variant_access.a #${WEBRTC_LIBS}/third_party/abseil-cpp/abseil-cpp/absl/types/libabsl_absl_bad_optional_access.a  


add_subdirectory(webrtc)
