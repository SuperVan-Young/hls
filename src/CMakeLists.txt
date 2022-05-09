message("Inside subdirectory src")

# libio
add_library(libio io.cpp)

# algorithm libs
add_subdirectory(allocate)
add_subdirectory(schedule)