cmake_minimum_required(VERSION 3.16)
project(hls)

add_subdirectory(src)

add_executable(hls src/main.cxx)
