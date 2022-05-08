message("Inside subdirectory src/schedule")

add_library(libschedule topo.cpp)

target_include_directories(libschedule PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)