cmake_minimum_required(VERSION 3.16)
project(hls)

SET(CMAKE_BUILD_TYPE "Debug") # for debugging, clear on handin

# third party libraries
include(FetchContent)

FetchContent_Declare(
    lpsolve
    URL https://sourceforge.net/projects/lpsolve/files/lpsolve/5.5.2.5/lp_solve_5.5.2.5_dev_ux64.tar.gz
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ext/lpsolve
)
FetchContent_MakeAvailable(lpsolve)

# find_library(LIB_LPSOLVE liblpsolve55.so ${CMAKE_SOURCE_DIR}/ext/lpsolve)
# message(${LIB_LPSOLVE})
link_directories(${CMAKE_SOURCE_DIR}/ext/lpsolve)
include_directories(${CMAKE_SOURCE_DIR}/ext/lpsolve)


add_subdirectory(src)

# install application to build
add_executable(hls src/main.cpp)
target_link_libraries(hls PRIVATE libhls)