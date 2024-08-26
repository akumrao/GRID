cmake_minimum_required(VERSION 3.0)

project(sdptransform VERSION 1.2.7)

# For CMake >= 3.1.
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
# For CMake < 3.1.
add_compile_options(-std=c++11)

#subdirs(test readme-helper)

#include_directories(./include  ../../../json/include/json/)

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

target_include_directories(sdptransform PRIVATE ${sdptransform_SOURCE_DIR}/include  /workspace/MagicAI/src/json/include/json/)


#install(TARGETS sdptransform DESTINATION lib)
#install(FILES ${HEADER_FILES} DESTINATION include/sdptransform)
