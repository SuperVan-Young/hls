cmake_minimum_required(VERSION 3.16)
project(hls)

SET(CMAKE_BUILD_TYPE "Debug") # for debugging, clear on handin

# third party libraries
include(FetchContent)

FetchContent_Declare(
    lpsolve
    URL https://sourceforge.net/projects/lpsolve/files/lpsolve/5.5.2.11/lp_solve_5.5.2.11_source.tar.gz
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ext/lpsolve
)

add_subdirectory(src)

# install application to build
add_executable(hls src/main.cpp)
target_link_libraries(hls PRIVATE libhls)