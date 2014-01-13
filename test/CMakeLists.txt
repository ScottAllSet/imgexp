#In Qt Creator, run cmake with "-DCMAKE_BUILD_TYPE=Debug" to debug.

cmake_minimum_required (VERSION 2.8)

project(imgexptest CXX)

find_package(Boost 1.55.0 COMPONENTS filesystem REQUIRED)

file(TO_CMAKE_PATH $ENV{GTEST_DIR} gtest_dir)
include_directories(include ${Boost_INCLUDE_DIR} ${gtest_dir}/include)
link_directories(${Boost_LIBRARY_DIR} ${gtest_dir} ${CMAKE_CURRENT_SOURCE_DIR}../../build)
add_executable(imgexptest src/imgexptest.cpp)

if(MSVC)
	set(gtest_main_lib gtest_main.lib)
	set(gtest_lib gtest.lib )
	set(imgexp_lib imgexp)
else()
	set(gtest_main_lib libgtest_main.a)
	set(gtest_lib libgtest.a)
	set(imgexp_lib imgexp.a)
endif()

target_link_libraries(imgexptest 
	${Boost_LIBRARIES}
	debug ${imgexp_lib}
	optimized ${imgexp_lib}
	debug ${gtest_dir}/Debug/${gtest_lib}
	optimized ${gtest_dir}/Release/${gtest_lib}
	debug ${gtest_dir}/Debug/${gtest_main_lib}
	optimized ${gtest_dir}/Release/${gtest_main_lib}
	)
