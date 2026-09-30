cmake_minimum_required(VERSION 3.15)
project(SupabaseWebRTCHost CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ------------------------------------------------------------------
# 1. DEPENDENCY INJECTOR (CONAN & SYSTEM PACKAGES)
# ------------------------------------------------------------------
# Find Conan packages
find_package(google-webrtc REQUIRED)
find_package(nlohmann_json REQUIRED)

# Find native system libwebsockets library (installed via apt)
find_library(LWS_LIBRARY NAMES websockets REQUIRED)
find_path(LWS_INCLUDE_DIR NAMES libwebsockets.h REQUIRED)

# Platform-specific preprocessor macros required by WebRTC
if(WIN32)
    add_compile_definitions(WEBRTC_WIN)
    set(SYSTEM_DEPENDENT_LIBS ws2_32 winmm dmoguids msdmo secur32)
elseif(APPLE)
    add_compile_definitions(WEBRTC_MAC WEBRTC_POSIX)
    set(SYSTEM_DEPENDENT_LIBS "-framework Foundation -framework Security")
else() # Linux Platforms
    add_compile_definitions(WEBRTC_LINUX WEBRTC_POSIX)
    set(SYSTEM_DEPENDENT_LIBS pthread dl rt OpenSSL::SSL OpenSSL::Crypto)
endif()

# ------------------------------------------------------------------
# 2. BUILD TARGET COMPILATION & MAPPING
# ------------------------------------------------------------------
add_executable(supabase_host host.cpp)

# Map include paths for libwebsockets manually
target_include_directories(supabase_host PRIVATE ${LWS_INCLUDE_DIR})

# Link libraries together
target_link_libraries(supabase_host PRIVATE
    google-webrtc::google-webrtc
    nlohmann_json::nlohmann_json
    ${LWS_LIBRARY}
    ${SYSTEM_DEPENDENT_LIBS}
)
