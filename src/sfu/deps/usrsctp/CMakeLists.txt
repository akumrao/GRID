cmake_minimum_required(VERSION 3.4.1)


set(TEST_CC_FILTER ".*gunit.cc|.*/mock/.*|.*mock_.*|.*/mocks/.*|.*fake.*|.*/test/.*|.*/tests/.*|.*_test_.*|.*unittest.*|.*/end_to_end_tests/.*|.*_test.cc|.*_tests.cc|.*_integrationtest.cc|.*_perftest.cc|.*test_utils.cc|.*testutils.cc|.*testclient.cc|.*test.c")

add_definitions(-DSCTP_PROCESS_LEVEL_LOCKS -DSCTP_SIMPLE_ALLOCATOR -DSCTP_USE_OPENSSL_SHA1 -D__Userspace__ -D__Userspace_os_Linux -D_GNU_SOURCE -DNON_WINDOWS_DEFINE)

include_directories(
    #${WEBRTC_REPO}/third_party/boringssl/src/include
     usrsctp/usrsctplib
)

file(GLOB_RECURSE src_usrsctp
     usrsctp/usrsctplib/*.c
)
list(FILTER src_usrsctp EXCLUDE REGEX ${TEST_CC_FILTER})

add_library(usrsctp STATIC
    ${src_usrsctp}
)

if (${ANDROID_ABI} STREQUAL "x86_64")

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g")

else(${ANDROID_ABI} STREQUAL "mipsel")
 
set(CMAKE_BUILD_TYPE MinSizeRel)

endif()


if (CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
    target_compile_options(usrsctp PRIVATE -Os  -s -Wl,-s -Wl,--gc-sections -fpermissive )
else()
#target_compile_options(usrsctp PRIVATE -fpermissive -DNDEBUG)
endif()