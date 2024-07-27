
cmake_minimum_required(VERSION 3.7)

if (${ANDROID_ABI} STREQUAL "x86_64")

SET(SSL_ROOT "/workspace/MagicAI/src/openssl/buildx64")
SET(SSL_LIB "/workspace/MagicAI/src/openssl/buildx64/lib")

SET(WEBRTC_REPO "/workspace/MagicAI/src/broadcast/main/webrtc/src")


else(${ANDROID_ABI} STREQUAL "mipsel")
 
SET(SSL_ROOT "/workspace/MagicAI/src/openssl/buildt31")
SET(SSL_LIB "/workspace/MagicAI/src/openssl/buildt31/lib")
SET(T31SDK "/workspace/adappt/T31/ISVP-T31-1.1.6-20221229/software/Ingenic-SDK-T31-1.1.6-20221229/sdk/5.4.0")

SET(WEBRTC_REPO "/workspace/MagicAI/src/broadcast/main/webrtc/src")


endif()


set(TEST_CC_FILTER ".*gunit.cc|.*/mock/.*|.*mock_.*|.*/mocks/.*|.*fake.*|.*/test/.*|.*/tests/.*|.*_test_.*|.*unittest.*|.*/end_to_end_tests/.*|.*_test.cc|.*_tests.cc|.*_integrationtest.cc|.*_perftest.cc|.*test_utils.cc|.*testutils.cc|.*testclient.cc|.*test.c")

set(OTHER_PLATFORM_CC_FILTER ".*_chromeos.cc|.*_freebsd.cc|.*_fuchsia.cc|.*/fuchsia/.*|.*_ios.cc|.*_ios.mm|.*/ios/.*|.*_mac.cc|.*_mac.mm|.*/mac/.*|.*_openbsd.cc|.*_win.cc|.*/win/.*|.*win32.*|.*/windows/.*|.*sse.cc|.*sse2.cc|.*_mips.cc|.*_mips.c")


set(arm_PLATFORM_CC_FILTER ".*fir_filter_neon.cc|.*sinc_resampler_neon.cc|.*cross_correlation_neon.c|.*downsample_fast_neon.c|.*min_max_operations_neon.c")




#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -g")
set(CMAKE_CXX_STANDARD 14)
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




#target_compile_definitions(runWebrtc PRIVATE HTTPSSL  WEBRTC_POSIX NDEBUG )


set(INC "/workspace/MagicAI/src/qrcode/include")


if (${ANDROID_ABI} STREQUAL "x86_64")

target_include_directories(runWebrtc PRIVATE   ${INC} ../../signal/include ../../facedetec/include/ ${SSL_ROOT}/include  ../include  ../../http/include/ ../../base/include/ ../../net/include/ ../../libuv/include ../../libuv/src ../../../build/ ../../http_parser/ ../webrtc/include ../include/x64  )

else(${ANDROID_ABI} STREQUAL "mipsel")
 
target_include_directories(runWebrtc PRIVATE  ${INC} ../../signal/include ../../facedetec/include/ ../../facedetec/include/t31/  ${SSL_ROOT}/include  ../include  ../../http/include/ ../../base/include/ ../../net/include/ ../../libuv/include ../../libuv/src ../../../build/ ../../http_parser/ ../webrtc/include   ../include/x64    ../include/t31 ../../facedetec/xailient-sdk-HD/xailient-sdk-2.1.0.2024/include/ ${T31SDK}/include ${T31SDK}/samples/libimp-sample/ ${JZDL_SDK_FOLDER_INC})

endif()




#target_compile_options(runWebrtc PRIVATE -Wno-return-type -Wno-format-overflow -Wno-format -Wno-unused-but-set-variable -Wno-write-strings -no-pie)
target_compile_features(runWebrtc PRIVATE cxx_std_14)

target_link_directories(runWebrtc PRIVATE  ../../facedetec/xailient-sdk-HD/xailient-sdk-2.1.0.2024/lib  ${T31SDK}/lib ${INSTALLPATH}/mips-linux-gnu/lib  /workspace/magik-toolkit/InferenceKit/jzdl/5.4.0/lib/uclibc ${JZDL_SDK_FOLDER_LIB})


add_subdirectory( webrtc/third_party)


if (${ANDROID_ABI} STREQUAL "x86_64")

target_link_libraries(runWebrtc PRIVATE  rnnoise
    srtp   usrsctp  event    absl::optional    absl::variant    absl::strings
    pffft pthread m dl z  ${SSL_LIB}/libssl.a ${SSL_LIB}/libcrypto.a xailientsdk xailientsdkutils  )

else(${ANDROID_ABI} STREQUAL "mipsel")
 
target_link_libraries(runWebrtc PRIVATE  rnnoise
    srtp   usrsctp  event  absl::optional  absl::variant   absl::strings 
    pffft  rt m dl -pthread atomic  ${SSL_LIB}/libssl.a ${SSL_LIB}/libcrypto.a  ${T31SDK}/lib/uclibc/libalog.a ${T31SDK}/lib/uclibc/libimp.a  /workspace/MagicAI/src/qrcode/rgbQr/zbar.a xailientsdk xailientsdkutils jzdl.m )

endif()





set(CMAKE_C_COMPILER_WORKS 1)

set(CMAKE_CXX_COMPILER_WORKS 1)


#SET(ANDROID_ABI "mipsel")

# Use C++11
# Require (at least) it
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Don't use e.g. GNU extension (like -std=gnu++11) for portability
set(CMAKE_CXX_EXTENSIONS OFF)

#set(CMAKE_POSITION_INDEPENDENT_CODE ON)


set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNDEBUG"  )
#set( CMAKE_EXPORT_COMPILE_COMMANDS ON )
#set(THREADS_PREFER_PTHREAD_FLAG TRUE)
#set(CMAKE_THREAD_PREFER_PTHREAD ON)


set(CMAKE_THREAD_PREFER_PTHREAD ON)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)




# add at first, avoid header search path, definition chaos
#add_subdirectory(third_party)



file(GLOB_RECURSE header_api
     #${webrtc_build_dir}/gen/sdk/android/generated_peerconnection_jni/*.h
)

file(GLOB_RECURSE src_api
    ${WEBRTC_REPO}/api/*.cc
)
list(FILTER src_api EXCLUDE REGEX ${TEST_CC_FILTER})
list(FILTER src_api EXCLUDE REGEX
    ".*echo_canceller3_config_json.cc|.*default_task_queue_factory_gcd.cc|.*default_task_queue_factory_stdlib.cc|.*default_task_queue_factory_win.cc|.*video_stream_decoder_create.cc"
)

list(FILTER src_api EXCLUDE REGEX
    "./audio_codecs/opus/*|.*/video_codecs/builtin_video_decoder_factory.cc|.*/video_codecs/builtin_video_encoder_factory.cc|.*/video_codecs/vp8*"
)



file(GLOB_RECURSE src_audio
    ${WEBRTC_REPO}/audio/*.cc
)
list(FILTER src_audio EXCLUDE REGEX ${TEST_CC_FILTER})

file(GLOB_RECURSE src_call
    ${WEBRTC_REPO}/call/*.cc
)
list(FILTER src_call EXCLUDE REGEX ${TEST_CC_FILTER})
list(APPEND src_call ${WEBRTC_REPO}/call/fake_network_pipe.cc)

file(GLOB_RECURSE src_common_audio
    ${WEBRTC_REPO}/common_audio/*.c
    ${WEBRTC_REPO}/common_audio/*.cc
)
list(FILTER src_common_audio EXCLUDE REGEX ${TEST_CC_FILTER})
list(FILTER src_common_audio EXCLUDE REGEX ${OTHER_PLATFORM_CC_FILTER})

list(FILTER src_common_audio EXCLUDE REGEX ${arm_PLATFORM_CC_FILTER}) 


file(GLOB_RECURSE src_common_video
    ${WEBRTC_REPO}/common_video/*.cc
)
list(FILTER src_common_video EXCLUDE REGEX ${TEST_CC_FILTER})

file(GLOB_RECURSE src_logging
    ${WEBRTC_REPO}/logging/rtc_event_log/encoder/*.cc
    ${WEBRTC_REPO}/logging/rtc_event_log/events/*.cc
    ${WEBRTC_REPO}/logging/rtc_event_log/output/*.cc
    ${WEBRTC_REPO}/logging/rtc_event_log/ice_logger.cc
    ${WEBRTC_REPO}/logging/rtc_event_log/rtc_event_log.cc
    ${WEBRTC_REPO}/logging/rtc_event_log/rtc_event_log_factory.cc
    ${WEBRTC_REPO}/logging/rtc_event_log/rtc_event_log_impl.cc
    ${WEBRTC_REPO}/logging/rtc_event_log/rtc_stream_config.cc
    #${WEBRTC_REPO}/${WEBRTC_BUILD_DIR}/gen/logging/*.cc
)
list(FILTER src_logging EXCLUDE REGEX ${TEST_CC_FILTER})

list(FILTER src_logging EXCLUDE REGEX ".*encoder/rtc_event_log_encoder_legacy.cc|.*encoder/rtc_event_log_encoder_new_format.cc")


file(GLOB_RECURSE src_media
    ${WEBRTC_REPO}/media/*.cc
)
list(FILTER src_media EXCLUDE REGEX ${TEST_CC_FILTER})
list(FILTER src_media EXCLUDE REGEX ".*fakertp.cc")
list(FILTER src_media EXCLUDE REGEX ".*/engine/internal_decoder_factory.cc|.*/engine/internal_encoder_factory.cc")


file(GLOB_RECURSE src_modules
    ${WEBRTC_REPO}/modules/*.c
    ${WEBRTC_REPO}/modules/*.cc
    #${WEBRTC_REPO}/${WEBRTC_BUILD_DIR}/gen/modules/*.cc
)
list(FILTER src_modules EXCLUDE REGEX ${TEST_CC_FILTER})
list(FILTER src_modules EXCLUDE REGEX ${OTHER_PLATFORM_CC_FILTER})
list(FILTER src_modules EXCLUDE REGEX 
    ".*/desktop_capture/.|.*include/audio_device_factory.cc|.*bwe_simulations.cc|.*/audio_coding/neteq/tools/.*|.*/remote_bitrate_estimator/tools/.*|.*rnn_vad_tool.cc|.*null_aec_dump_factory.cc|.*h264_color_space.cc|.*vp9_noop.cc|.*h264_decoder_impl.cc|.*h264_encoder_impl.cc|.*/android/aaudio_.*|.*ensure_initialized.cc|.*fixed_gain_controller.cc|.*click_annotate.cc"
)

list(FILTER src_modules EXCLUDE REGEX 
    ".*codecs/isac/fix/source/entropy_coding_neon.c|.*codecs/isac/fix/source/filterbanks_neon.c|.*codecs/isac/fix/source/filters_neon.c|.*codecs/isac/fix/source/lattice_neon.c|.*codecs/isac/fix/source/transform_neon.c|.*audio_device/android/*|.*aec/aec_core_neon.cc|.*audio_processing/aecm/aecm_core_neon.cc|.*audio_processing/ns/nsx_core_neon.c|.*audio_processing/utility/ooura_fft_neon.cc|.*utility/source/jvm_android.cc|.*utility/source/helpers_android.cc|.*util/denoiser_filter_neon.cc"
)

list(FILTER src_modules EXCLUDE REGEX 
    ".*audio_coding/codecs/opus/*|.*audio_device/linux/*|.*/audio_processing/aec_dump/*|.*video_coding/codecs/vp8/*|.*video_coding/codecs/vp9/*"
)




file(GLOB_RECURSE src_ortc
    ${WEBRTC_REPO}/ortc/*.cc
)
list(FILTER src_ortc EXCLUDE REGEX ${TEST_CC_FILTER})

file(GLOB_RECURSE src_p2p
    ${WEBRTC_REPO}/p2p/*.cc
)
list(FILTER src_p2p EXCLUDE REGEX ${TEST_CC_FILTER})

file(GLOB_RECURSE src_pc
    ${WEBRTC_REPO}/pc/*.cc
)
list(FILTER src_pc EXCLUDE REGEX ${TEST_CC_FILTER})
list(FILTER src_pc EXCLUDE REGEX 
    ".*peer_connection_wrapper.cc"
)

file(GLOB_RECURSE src_rtc_base
    ${WEBRTC_REPO}/rtc_base/*.cc
)
list(FILTER src_rtc_base EXCLUDE REGEX ${TEST_CC_FILTER})
list(FILTER src_rtc_base EXCLUDE REGEX ${OTHER_PLATFORM_CC_FILTER})
list(FILTER src_rtc_base EXCLUDE REGEX 
    ".*mac_utils.cc|.*mac_ifaddrs_converter.cc|.*test_echo_server.cc|.*task_queue_gcd.cc|.*task_queue_stdlib.cc|.*task_queue_for_test.cc|.*virtual_socket_server.cc|.*nat_socket_factory.cc|.*nat_server.cc|.*strings/json.cc"
)

list(FILTER src_rtc_base EXCLUDE REGEX 
    ".*system/thread_registry.cc|.*system/warn_current_thread_is_deadlocked.cc"
)





file(GLOB_RECURSE src_sdk
   # ${WEBRTC_REPO}/sdk/android/*.cc
    ${WEBRTC_REPO}/sdk/media_constraints.cc
)
list(FILTER src_sdk EXCLUDE REGEX ${TEST_CC_FILTER})
list(FILTER src_sdk EXCLUDE REGEX 
    ".*null_audio.cc|.*null_media.cc|.*null_video.cc"
)

list(FILTER src_sdk EXCLUDE REGEX 
    ".*sdk/android/*"
)


file(GLOB_RECURSE src_stats
    ${WEBRTC_REPO}/stats/*.cc
)
list(FILTER src_stats EXCLUDE REGEX ${TEST_CC_FILTER})

file(GLOB_RECURSE src_system_wrappers
    ${WEBRTC_REPO}/system_wrappers/*.cc
)
list(FILTER src_system_wrappers EXCLUDE REGEX ${TEST_CC_FILTER})
list(FILTER src_system_wrappers EXCLUDE REGEX ${OTHER_PLATFORM_CC_FILTER})

file(GLOB_RECURSE src_video
    ${WEBRTC_REPO}/video/*.cc
)
list(FILTER src_video EXCLUDE REGEX ${TEST_CC_FILTER})
list(FILTER src_video EXCLUDE REGEX 
    ".*_loopback.cc|.*replay.cc|.*video_analyzer.cc|.*video_loopback_main.cc|.*video_stream_decoder_impl.cc"
)




file(GLOB_RECURSE HEADERS_WEBRTC
     ${WEBRTC_REPO}*.h
)

target_sources(runWebrtc PRIVATE
    ${src_api}
    ${src_audio}
    ${src_call}
    ${src_common_audio}
    ${src_common_video}
    ${src_logging}
    ${src_media}
    ${src_modules}
    ${src_ortc}
    ${src_p2p}
    ${src_pc}
    ${src_rtc_base}
    ${src_sdk}
    ${src_stats}
    ${src_system_wrappers}
    ${src_video}
    ${header_api}

    PUBLIC  

    ${HEADERS_WEBRTC}
    
)


if (${ANDROID_ABI} STREQUAL "armeabi-v7a")
    target_compile_definitions(runWebrtc PRIVATE -DWEBRTC_USE_BUILTIN_ISAC_FIX=1 -DWEBRTC_USE_BUILTIN_ISAC_FLOAT=0)
else()
    target_compile_definitions(runWebrtc PRIVATE -DWEBRTC_USE_BUILTIN_ISAC_FIX=0 -DWEBRTC_USE_BUILTIN_ISAC_FLOAT=1)
endif()



if (${ANDROID_ABI} STREQUAL "arm64-v8a")
    target_compile_definitions(runWebrtc PRIVATE -DWEBRTC_ARCH_ARM64 -DWEBRTC_HAS_NEON)
elseif (${ANDROID_ABI} STREQUAL "armeabi-v7a")
    target_compile_definitions(runWebrtc PRIVATE -DWEBRTC_ARCH_ARM -DWEBRTC_ARCH_ARM_V7 -DWEBRTC_HAS_NEON -DARVIND)
endif()





target_compile_definitions(runWebrtc PRIVATE -DHAVE_NETINET_IN_H  -DWEBRTC_APM_DEBUG_DUMP=0  -DWEBRTC_INTELLIGIBILITY_ENHANCER=0 -DWEBRTC_NS_FIXED -DWEBRTC_CODEC_ILBC -DWEBRTC_CODEC_OPUS -DWEBRTC_OPUS_SUPPORT_120MS_PTIME=1 -DWEBRTC_CODEC_ISAC -DWEBRTC_CODEC_RED 
-DWEBRTC_POSIX=1 -DWEBRTC_LINUX=1 -DNDEBUG 
-DWEBRTC_INCLUDE_INTERNAL_AUDIO_DEVICE
-DHAVE_PTHREAD -DHAVE_SCTP -DHAVE_WEBRTC_VIDEO -DHAVE_WEBRTC_VOICE
-DNO_TCMALLOC=1
-DWEBRTC_NON_STATIC_TRACE_EVENT_HANDLERS=0
-DHTTPSSL  -DWEBRTC_POSIX -DNDEBUG 

)

target_include_directories(runWebrtc PRIVATE  

    ${WEBRTC_REPO}

    #/workspace/webrtc/src

    #${WEBRTC_REPO}/${WEBRTC_BUILD_DIR}/gen

    ${WEBRTC_REPO}/third_party/abseil-cpp
    #${WEBRTC_REPO}/third_party/boringssl/src/include
    /workspace/MagicAI/src/openssl/buildx64/include
    ${WEBRTC_REPO}/third_party/icu/source/common
    ${WEBRTC_REPO}/sdk/libs/ffmpeg/include
    ${WEBRTC_REPO}/third_party/jsoncpp/source/include
    ${WEBRTC_REPO}/third_party/libsrtp/config
    ${WEBRTC_REPO}/third_party/libsrtp/crypto/include
    ${WEBRTC_REPO}/third_party/libsrtp/include
    #${WEBRTC_REPO}/third_party/libyuv/include
    #${WEBRTC_REPO}/third_party/libvpx/source/libvpx
    #${WEBRTC_REPO}/third_party/opus/src/include
    #${WEBRTC_REPO}/third_party/protobuf/src
    ${WEBRTC_REPO}/third_party/usrsctp/usrsctplib
    #${webrtc_build_dir}/gen/sdk/android/generated_peerconnection_jni
    #${webrtc_build_dir}/gen/sdk/android/generated_logging_jni
    #${webrtc_build_dir}/gen/sdk/android/generated_native_api_jni/

    #${WEBRTC_REPO}/third_party/android_ndk/sysroot/usr/include
) 




if (${ANDROID_ABI} STREQUAL "x86_64")

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g")

else(${ANDROID_ABI} STREQUAL "mipsel")
 
set(CMAKE_BUILD_TYPE MinSizeRel)

endif()


if (CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
    target_compile_options(runWebrtc PRIVATE -Os -flto -s -Wl,-s -Wl,--gc-sections -fpermissive -DNDEBUG)
else()
target_compile_options(runWebrtc PRIVATE -fpermissive -DNDEBUG)
endif()

