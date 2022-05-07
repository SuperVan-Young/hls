cmake_minimum_required(VERSION 3.16)
project(hls)

set(CMAKE_BUILD_TYPE "Debug")

include_directories(include)

add_subdirectory(src)

add_executable(hls src/main.cpp)
target_link_libraries(hls libio)
target_link_libraries(hls libalgo)

add_subdirectory(test)
