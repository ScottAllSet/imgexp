#In Qt Creator, run cmake with "-DCMAKE_BUILD_TYPE=Debug" to debug.

cmake_minimum_required(VERSION 2.8.7)
project(expbuilder CXX)

cmake_policy(SET CMP0020 NEW)
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
find_package(Qt5Widgets 5.2 REQUIRED)
include_directories(include)
qt5_wrap_ui(ui_headers ui/mainwindow.ui)

if(WIN32)
	set(GUI_TYPE WIN32)
endif(WIN32)
if (APPLE)
	set(GUI_TYPE MACOSX_BUNDLE)
endif (APPLE)

add_executable(expbuilder ${GUI_TYPE} src/main.cpp src/mainwindow.cpp include/mainwindow.h ${ui_headers})
qt5_use_modules(expbuilder Widgets)
