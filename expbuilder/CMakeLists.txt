#For debug, run cmake with "-DCMAKE_BUILD_TYPE=Debug"

cmake_minimum_required(VERSION 2.8.7)
project(expbuilder CXX)

cmake_policy(SET CMP0020 NEW)

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)


set(Qt5Widgets_DIR "$ENV{QTDIR}\\qtbase\\lib\\cmake\\Qt5Widgets")

find_package(Qt5Widgets 5.2 REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

include_directories(include)

qt5_wrap_ui(ui_headers ui/mainwindow.ui)

add_executable(expbuilder src/main.cpp src/mainwindow.cpp include/mainwindow.h ${ui_headers})

qt5_use_modules(expbuilder Widgets)
