#run cmake with "-DCMAKE_BUILD_TYPE=Debug"

cmake_minimum_required (VERSION 2.8)

project(imgexp CXX)

add_definitions(-DJSON_IS_AMALGAMATION)

set(Boost_DEBUG 1)

find_package(Boost 1.55.0)

#uncomment to enable verbose compilation
#add_definitions(-v)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

file(TO_CMAKE_PATH $ENV{JSONCPP_DIR} jsoncpp_dir)

include_directories(include ${Boost_INCLUDE_DIR} ${jsoncpp_dir})

set(jsoncpp_source ${jsoncpp_dir}/jsoncpp.cpp)

set(imgexp_source src/imgexp.cpp)

set(imgexp_include include/config.h include/imgexp.h)

add_library(imgexp SHARED ${jsoncpp_source} ${imgexp_source} ${imgexp_include})
