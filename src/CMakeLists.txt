message("Inside subdirectory src")

# libio
add_library(libio io.cpp)

# libalgo
add_subdirectory(schedule)

add_library(libalgo allocate.cpp)
target_link_libraries(libalgo libschedule)