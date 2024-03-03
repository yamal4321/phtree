cmake_minimum_required(VERSION 3.20)

project(ph_tree)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/libmorton)

enable_testing()
add_subdirectory(bench)
#add_subdirectory(test)
#add_subdirectory(visualization)
