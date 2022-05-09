message("Inside subdirectory src/schedule")

add_library(liballocate allocate.cpp)

target_include_directories(liballocate PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)