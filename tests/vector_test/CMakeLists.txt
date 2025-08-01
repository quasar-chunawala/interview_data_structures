cmake_minimum_required(VERSION 3.27)

project(vector_test)

# set the C++ language standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED true)

set(SOURCES 
    ./vector_test.cpp
)

set(INCLUDE_DIRECTORIES
    ../../include/vector/
    ${gtest_SOURCE_DIR}/include
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

add_executable(vector_test ${SOURCES})

target_include_directories(vector_test PUBLIC ${INCLUDE_DIRECTORIES})

target_link_libraries(vector_test gtest gtest_main)

# Add AddressSanitizer and gcov flags conditionally
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Building the vector_test target in Debug mode...")
    if(MSVC)
        target_compile_options(vector_test PRIVATE /fsanitize=address /Zi /MD)
        target_link_options(vector_test PRIVATE /fsanitize=address)
    else()
        target_compile_options(vector_test PRIVATE --coverage -fsanitize=address -g)
        target_link_options(vector_test PRIVATE --coverage -fsanitize=address)
    endif()
endif()

# Discover and register Google Test cases
include(GoogleTest)
gtest_discover_tests(vector_test)
