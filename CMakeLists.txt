cmake_minimum_required(VERSION 3.10)

project(
    DSA-C
)

# set(BUILD_SHARED_LIBS off)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_library(btree btree.c)
add_library(dict dict.c)
add_library(set set.c)
add_library(deque deque.c)

add_subdirectory(test)
