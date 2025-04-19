project(usrsctp
	VERSION 0.21.2
	LANGUAGES C)

cmake_minimum_required(VERSION 3.4.1)

message(STATUS "arvind" ${WEBRTC_REPO}/deps/usrsctp/usrsctplib)


set(TEST_CC_FILTER ".*gunit.cc|.*/mock/.*|.*mock_.*|.*/mocks/.*|.*fake.*|.*/test/.*|.*/tests/.*|.*_test_.*|.*unittest.*|.*/end_to_end_tests/.*|.*_test.cc|.*_tests.cc|.*_integrationtest.cc|.*_perftest.cc|.*test_utils.cc|.*testutils.cc|.*testclient.cc|.*test.c")


add_definitions(-DSCTP_PROCESS_LEVEL_LOCKS -DSCTP_SIMPLE_ALLOCATOR -DSCTP_USE_OPENSSL_SHA1 -D__Userspace__ -D__Userspace_os_Linux -D_GNU_SOURCE -DNON_WINDOWS_DEFINE)

include_directories(
    #${WEBRTC_REPO}/deps/boringssl/src/include
    ${WEBRTC_REPO}/deps/usrsctp/usrsctplib
)

file(GLOB_RECURSE src_usrsctp
    ${WEBRTC_REPO}/deps/usrsctp/usrsctplib/*.c
)

list(FILTER src_usrsctp EXCLUDE REGEX ${TEST_CC_FILTER})

add_library(usrsctp STATIC
    ${src_usrsctp}
)


SET_TARGET_PROPERTIES( usrsctp 
    PROPERTIES 
       LINKER_LANGUAGE C
)
