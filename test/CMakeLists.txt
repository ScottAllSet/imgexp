#run cmake with "-DCMAKE_BUILD_TYPE=Debug"

cmake_minimum_required (VERSION 2.8)

project(imgexptext CXX)

add_definitions(-DJSON_IS_AMALGAMATION)

#uncomment to enable verbose compilation
#add_definitions(-v)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

file(TO_CMAKE_PATH $ENV{GTEST_DIR} gtest_dir)
file(TO_CMAKE_PATH $ENV{BOOST_DIR} boost_dir)
file(TO_CMAKE_PATH $ENV{JSONCPP_DIR} jsoncpp_dir)

include_directories(include ${gtest_dir}/include ${boost_dir} ${jsoncpp_dir})

link_directories(${gtest_dir} ${CMAKE_CURRENT_SOURCE_DIR}../../build)

#set(jsoncpp_source ${jsoncpp_dir}/jsoncpp.cpp)

add_executable(imgexptest src/imgexptest.cpp)
target_link_libraries(imgexptest libgtest.a libgtest_main.a libimgexp.dll.a)
