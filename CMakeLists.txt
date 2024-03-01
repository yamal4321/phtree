cmake_minimum_required(VERSION 3.20)

project(ph_tree)

set(CMAKE_BUILD_TYPE "Release")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/libmorton)

enable_testing()
#add_subdirectory(bench)
add_subdirectory(test)
#add_subdirectoryadd_subdirectory(visualization)
