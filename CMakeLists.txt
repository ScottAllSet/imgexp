#In Qt Creator, run cmake with "-DCMAKE_BUILD_TYPE=Debug" to debug.

project(imgexp)

#common
cmake_policy(SET CMP0020 NEW)
cmake_minimum_required (VERSION 2.8.7)
add_definitions(-DJSON_IS_AMALGAMATION)
set(Boost_DEBUG 0)
find_package(Boost 1.55.0)

include(CheckCXXCompilerFlag)

check_cxx_compiler_flag(-std=c++11 HAS_STD_CXX11)
if(HAS_STD_CXX11)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

file(TO_CMAKE_PATH $ENV{JSONCPP_DIR} jsoncpp_dir)
set(jsoncpp_source ${jsoncpp_dir}/jsoncpp.cpp)
include_directories(include ${Boost_INCLUDE_DIR} ${jsoncpp_dir})

#imgexp
set(imgexp_source src/imgexp.cpp)
set(imgexp_include include/config.h include/imgexp.h)
add_library(imgexp SHARED ${jsoncpp_source} ${imgexp_source} ${imgexp_include})

#imgexptest
add_subdirectory(test)

#expbuilder
add_subdirectory(expbuilder)