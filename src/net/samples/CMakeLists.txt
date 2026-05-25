cmake_minimum_required(VERSION 3.12.0)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

project(runUdp VERSION 1.7.7 LANGUAGES CXX)


set(TTYD_VERSION "${PROJECT_VERSION}")

set(CMAKE_CXX_STANDARD 17)

# Enforce the standard; if the compiler is too old, the build fails
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# set(CMAKE_C_STANDARD 17)
# set(CMAKE_C_STANDARD_REQUIRED ON)




list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/Modules)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)


if(WIN32)
	add_definitions(-DWIN32_LEAN_AND_MEAN)
	if(MSVC)
		add_definitions(-DNOMINMAX)
		add_definitions(-D_CRT_SECURE_NO_WARNINGS)
		add_compile_options(/W2)
	endif()

else()
add_definitions( -Wno-write-strings -Wno-missing-field-initializers -Wno-unused-parameter -Wno-unused-value   -Wno-extra -Wno-switch)
endif()

if(CMAKE_COMPILER_IS_GNUCC)
    add_compile_options(-Wno-unused-but-set-variable)
endif()



set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)




if(WIN32)
	link_libraries(  ws2_32) # winsock2
endif()


set(SOURCE_FILES testUdpServer.cpp)
set(SOURCE_FILES2 testUdpClient.cpp)
set(SOURCE_FILES3 testUdpServerToServer.cpp)


if(UNIX)
file(GLOB SOURCES   
    ../src/UdpSocket.cpp ../src/IP.cpp ../src/dns.cpp  ../../base/src/*.cpp ../../libuv/src/*.cpp ../../libuv/src/unix/*.cpp
  )
else()
file(GLOB SOURCES   
    ../src/UdpSocket.cpp  ../src/IP.cpp ../src/dns.cpp  ../../base/src/*.cpp 
  )
endif()


if(WIN32)
    list(APPEND LINK_LIBS shell32 ws2_32)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/app.rc.in ${CMAKE_CURRENT_BINARY_DIR}/app.rc @ONLY)
    list(APPEND SOURCE_FILES ${CMAKE_CURRENT_BINARY_DIR}/app.rc)
else()
    find_library(LIBUTIL NAMES util)
    if(LIBUTIL)
        list(APPEND LINK_LIBS util)
    endif()
endif()

if(WIN32)
add_subdirectory(../../libuv build )
endif()


if(WIN32)
list(APPEND LINK_LIBS uv_a  )
endif()

if(UNIX)
list(APPEND LINK_LIBS  dl Threads::Threads )
endif()



if(CAPI_STDCALL)
	add_compile_definitions( CAPI_STDCALL)
endif()


if(UNIX)
add_compile_options( -fpermissive -Wno-return-type -Wno-format-overflow -Wno-format -Wno-write-strings -no-pie)
endif()

if(WIN32)
#add_compile_options( PRIVATE )
endif()



if(NOT MSVC)
	add_compile_options( -Wall -Wextra)
endif()

if(WARNINGS_AS_ERRORS)
	if(MSVC)
		add_compile_options(  /WX)
	else()
		add_compile_options(  -Werror)
	endif()
endif()



include_directories(
           ../sctp/ ../pc/ ../dtls/ ../../http/include/ ../../http_parser/ ../../base/include/ ../../net/include/ ../../libuv/include ../../libuv/src ../../json/include/ ../../http_parser/ ../../signal/include/ )



add_executable(${PROJECT_NAME}_server ${SOURCE_FILES} ${SOURCES})
target_include_directories(${PROJECT_NAME}_server PUBLIC ${INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME}_server  ${LINK_LIBS} )
target_compile_definitions(${PROJECT_NAME}_server PUBLIC
    TTYD_VERSION="${TTYD_VERSION}"
    $<$<PLATFORM_ID:Windows>:_WIN32_WINNT=0xa00 WINVER=0xa00>
)


add_executable(${PROJECT_NAME}_client ${SOURCE_FILES2} ${SOURCES})
target_include_directories(${PROJECT_NAME}_client PUBLIC ${INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME}_client   ${LINK_LIBS})
target_compile_definitions(${PROJECT_NAME}_client PUBLIC
    TTYD_VERSION="${TTYD_VERSION}"
    $<$<PLATFORM_ID:Windows>:_WIN32_WINNT=0xa00 WINVER=0xa00>
)


add_executable(${PROJECT_NAME}_serverToServer ${SOURCE_FILES3} ${SOURCES})
target_include_directories(${PROJECT_NAME}_serverToServer PUBLIC ${INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME}_serverToServer   ${LINK_LIBS})
target_compile_definitions(${PROJECT_NAME}_serverToServer PUBLIC
    TTYD_VERSION="${TTYD_VERSION}"
    $<$<PLATFORM_ID:Windows>:_WIN32_WINNT=0xa00 WINVER=0xa00>
)
