# build lib

add_library(libhls STATIC)

# add interface
target_include_directories(libhls PUBLIC ${CMAKE_SOURCE_DIR}/include)

# add source files
aux_source_directory(data HLS_SOURCE_DATA)
aux_source_directory(allocate HLS_SOURCE_ALLOCATE)
aux_source_directory(schedule HLS_SOURCE_SCHEDULE)
aux_source_directory(bind HLS_SOURCE_BIND)

target_sources(
    libhls
    PRIVATE ${HLS_SOURCE_DATA}
    PRIVATE ${HLS_SOURCE_ALLOCATE}
    PRIVATE ${HLS_SOURCE_SCHEDULE}
    PRIVATE ${HLS_SOURCE_BIND}
)