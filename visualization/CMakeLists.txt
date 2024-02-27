cmake_minimum_required(VERSION 3.5)

project(ph_gui_knn VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/phnew)

set(PROJECT_SOURCES main.cpp widget.cpp widget.h)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libmorton)

add_executable(ph_gui ${PROJECT_SOURCES})

target_compile_options(ph_gui PRIVATE -O3)

target_link_libraries(ph_gui PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
