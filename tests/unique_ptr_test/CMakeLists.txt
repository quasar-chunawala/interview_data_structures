cmake_minimum_required(VERSION 3.25)

# Project
project(unique_ptr_test)

# Set the C++ language standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED 23)


# set include directories
set(INCLUDE_DIRECTORIES
    ${gtest_SOURCE_DIR}/include
    ../../include/unique_ptr/
)

# Add source files
set(SOURCE_FILES 
    unique_ptr_test.cpp
)

# Set output directory for all binaries
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}) # For static libraries

add_executable(unique_ptr_test ${SOURCE_FILES})

# Link Google Test libraries to the target
target_link_libraries(unique_ptr_test gtest gtest_main)

# Specify include directories for the target
target_include_directories(unique_ptr_test PUBLIC ${INCLUDE_DIRECTORIES})

# Add AddressSanitizer and gcov flags conditionally
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Building the unique_ptr_test target in Debug mode...")
    if(MSVC)
        target_compile_options(unique_ptr_test PRIVATE /fsanitize=address /Zi /MD)
        target_link_options(unique_ptr_test PRIVATE /fsanitize=address)
    else()
        target_compile_options(unique_ptr_test PRIVATE --coverage -fsanitize=address -g)
        target_link_options(unique_ptr_test PRIVATE --coverage -fsanitize=address)
    endif()
endif()

# Discover and register Google Test cases
include(GoogleTest)
gtest_discover_tests(unique_ptr_test)
