message("Inside subdirectory src/bind")

add_library(libbind bind.cpp)

target_include_directories(libbind PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)