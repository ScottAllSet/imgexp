#In Qt Creator, run cmake with "-DCMAKE_BUILD_TYPE=Debug" to debug.

cmake_minimum_required (VERSION 2.8)

project(imgexptest CXX)
file(TO_CMAKE_PATH $ENV{GTEST_DIR} gtest_dir)
include_directories(include ${gtest_dir}/include)
link_directories(${gtest_dir} ${CMAKE_CURRENT_SOURCE_DIR}../../build)
add_executable(imgexptest src/imgexptest.cpp)

if(MSVC)
	set(gtest_main_lib gtest_main.lib)
	set(gtest_lib gtest.lib )
else()
	set(gtest_main_lib libgtest_main.a)
	set(gtest_lib libgtest.a)
endif()

#TODO: add imgexp.lib
target_link_libraries(imgexptest 
	debug ${gtest_dir}/Debug/${gtest_lib}
	debug ${gtest_dir}/Debug/${gtest_main_lib}
	optimized ${gtest_dir}/Release/${gtest_lib}
	optimized ${gtest_dir}/Release/${gtest_main_lib}
	)
