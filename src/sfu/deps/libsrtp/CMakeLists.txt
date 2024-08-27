cmake_minimum_required(VERSION 3.4.1)


set(TEST_CC_FILTER ".*gunit.cc|.*/mock/.*|.*mock_.*|.*/mocks/.*|.*fake.*|.*/test/.*|.*/tests/.*|.*_test_.*|.*unittest.*|.*/end_to_end_tests/.*|.*_test.cc|.*_tests.cc|.*_integrationtest.cc|.*_perftest.cc|.*test_utils.cc|.*testutils.cc|.*testclient.cc|.*test.c")

include_directories(
    /workspace/MagicAI/src/openssl/buildt31/include/
    config
    libsrtp/crypto/include
    libsrtp/include
)

add_definitions(
    -DHAVE_CONFIG_H
    -DOPENSSL
    -DHAVE_STDLIB_H
    -DHAVE_STRING_H
    -DHAVE_STDINT_H
    -DHAVE_INTTYPES_H
    -DHAVE_INT16_T
    -DHAVE_INT32_T
    -DHAVE_INT8_T
    -DHAVE_UINT16_T
    -DHAVE_UINT32_T
    -DHAVE_UINT64_T
    -DHAVE_UINT8_T
    -DHAVE_ARPA_INET_H
    -DHAVE_NETINET_IN_H
    -DHAVE_SYS_TYPES_H
    -DHAVE_UNISTD_H
#    -DPACKAGE_STRING="libsrtp2 2.1.0-pre"
#    -DPACKAGE_VERSION="2.1.0-pre"
)

file(GLOB_RECURSE src_srtp
    libsrtp/*.c
)
list(FILTER src_srtp EXCLUDE REGEX ${TEST_CC_FILTER})

add_library(srtp STATIC
    ${src_srtp}
)


if (${ANDROID_ABI} STREQUAL "x86_64")

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g")

else(${ANDROID_ABI} STREQUAL "mipsel")
 
set(CMAKE_BUILD_TYPE MinSizeRel)

endif()


if (CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
    target_compile_options(srtp PRIVATE -Os  -s -Wl,-s -Wl,--gc-sections -fpermissive )
else()
#target_compile_options(srtp PRIVATE -fpermissive -DNDEBUG)
endif()
