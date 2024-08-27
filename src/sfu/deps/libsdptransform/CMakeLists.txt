cmake_minimum_required(VERSION 3.0)

project(sdptransform VERSION 1.2.7)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-std=c++14)

set(
	SOURCE_FILES
	src/grammar.cpp
	src/parser.cpp
	src/writer.cpp
)

set(
	HEADER_FILES
	include/sdptransform.hpp
)

add_library(sdptransform STATIC ${SOURCE_FILES} ${HEADER_FILES})

target_include_directories(sdptransform PRIVATE include ../../../json/include/json)



if (${ANDROID_ABI} STREQUAL "x86_64")

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g")

else(${ANDROID_ABI} STREQUAL "mipsel")
 
set(CMAKE_BUILD_TYPE MinSizeRel)

endif()


if (CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
    target_compile_options(sdptransform PRIVATE -Os -s -Wl,-s -Wl,--gc-sections -fpermissive -DNDEBUG )
else()
#target_compile_options(sdptransform PRIVATE -fpermissive -DNDEBUG)
endif()

