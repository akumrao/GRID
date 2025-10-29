project(srtp
    VERSION 0.21.2
    LANGUAGES C)

cmake_minimum_required(VERSION 3.4.1)

message(STATUS "arvind" ${WEBRTC_REPO}/deps/libsrtp)

set(TEST_CC_FILTER ".*gunit.cc|.*/mock/.*|.*mock_.*|.*/mocks/.*|.*fake.*|.*/test/.*|.*/tests/.*|.*_test_.*|.*unittest.*|.*/end_to_end_tests/.*|.*_test.cc|.*_tests.cc|.*_integrationtest.cc|.*_perftest.cc|.*test_utils.cc|.*testutils.cc|.*testclient.cc|.*test.c")


include_directories(
    ${WEBRTC_REPO}/deps/boringssl/src/include
    ${WEBRTC_REPO}/deps/libsrtp/config
    ${WEBRTC_REPO}/deps/libsrtp/crypto/include
    ${WEBRTC_REPO}/deps/libsrtp/include
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
    -DPACKAGE_STRING="libsrtp2 2.1.0-pre"
    -DPACKAGE_VERSION="2.1.0-pre"
)

file(GLOB_RECURSE src_srtp
    ${WEBRTC_REPO}/deps/libsrtp/*.c
)
list(FILTER src_srtp EXCLUDE REGEX ${TEST_CC_FILTER})

add_library(srtp STATIC
    ${src_srtp}
)


SET_TARGET_PROPERTIES(srtp 
    PROPERTIES 
       LINKER_LANGUAGE C
)
