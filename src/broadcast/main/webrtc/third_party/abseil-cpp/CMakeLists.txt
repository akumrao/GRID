cmake_minimum_required(VERSION 3.4.1)

message("WEBRTC_REPO", ${WEBRTC_REPO})

set(CMAKE_INSTALL_PREFIX /tmp)
add_subdirectory(${WEBRTC_REPO}/third_party/abseil-cpp abseil-cpp)
