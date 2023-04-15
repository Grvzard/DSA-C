cmake_minimum_required(VERSION 3.10)

project(
    DSA-C
)

set(BUILD_SHARED_LIBS on)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_library(btree SHARED btree.c)
add_library(dict SHARED dict.c)
add_library(set SHARED set.c)

add_subdirectory(test)
