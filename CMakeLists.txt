cmake_minimum_required(VERSION 3.16)
project(hls)

set(CMAKE_BUILD_TYPE "Debug")

include_directories(include)

add_subdirectory(src)

add_executable(hls src/main.cpp)
target_link_libraries(hls libio)
target_link_libraries(hls liballocate)
target_link_libraries(hls libschedule)
target_link_libraries(hls libbind)

add_subdirectory(test)
