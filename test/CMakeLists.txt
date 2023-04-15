cmake_minimum_required(VERSION 3.10)

include_directories(${PROJECT_SOURCE_DIR})

add_executable(set_test EXCLUDE_FROM_ALL set_test.c)
target_link_libraries(set_test set)

set_target_properties(set_test
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/test"
    )
