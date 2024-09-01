cmake_minimum_required(VERSION 3.0)

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
    target_compile_options(sdptransform PRIVATE -Os -s -Wl,-s -Wl,--gc-sections -fpermissive -DNDEBUG )
else()
#target_compile_options(sdptransform PRIVATE -fpermissive -DNDEBUG)
endif()

