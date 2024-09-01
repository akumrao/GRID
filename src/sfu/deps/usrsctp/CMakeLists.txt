cmake_minimum_required(VERSION 3.4.1)


set(TEST_CC_FILTER ".*gunit.cc|.*/mock/.*|.*mock_.*|.*/mocks/.*|.*fake.*|.*/test/.*|.*/tests/.*|.*_test_.*|.*unittest.*|.*/end_to_end_tests/.*|.*_test.cc|.*_tests.cc|.*_integrationtest.cc|.*_perftest.cc|.*test_utils.cc|.*testutils.cc|.*testclient.cc|.*test.c")

add_definitions(-DSCTP_PROCESS_LEVEL_LOCKS -DSCTP_SIMPLE_ALLOCATOR -DSCTP_USE_OPENSSL_SHA1 -D__Userspace__ -D__Userspace_os_Linux -D_GNU_SOURCE -DNON_WINDOWS_DEFINE)



file(GLOB_RECURSE src_usrsctp
     usrsctp/usrsctplib/*.c
)
list(FILTER src_usrsctp EXCLUDE REGEX ${TEST_CC_FILTER})

add_library(usrsctp STATIC
    ${src_usrsctp}
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



if (CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
    target_compile_options(usrsctp PRIVATE -Os  -s -Wl,-s -Wl,--gc-sections -fpermissive )
else()
#target_compile_options(usrsctp PRIVATE -fpermissive -DNDEBUG)
endif()

target_include_directories( usrsctp PRIVATE
 ${SSL_ROOT}/include  
 #/workspace/MagicAI/src/sfu/deps/openssl/build/openssl/arm64-v8a/include
  usrsctp/usrsctplib
)
