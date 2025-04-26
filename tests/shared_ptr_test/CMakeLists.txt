cmake_minimum_required(VERSION 3.25)

# Project
project(shared_ptr_test)

# Set the C++ language standard
set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED 26)

# Add googletest as a subproject
# add_subdirectory(../../ext/googletest)

# set include directories
set(INCLUDE_DIRECTORIES
    ../../ext/googletest/googletest/include
    ../../include/shared_ptr/
)

# Add source files
set(SOURCE_FILES 
    shared_ptr_test.cpp
)

add_executable(shared_ptr_test ${SOURCE_FILES})

# Link Google Test libraries to the target
target_link_libraries(shared_ptr_test gtest gtest_main)

# Specify include directories for the target
target_include_directories(shared_ptr_test PUBLIC ${INCLUDE_DIRECTORIES})

# Discover and register Google Test cases
include(GoogleTest)
gtest_discover_tests(shared_ptr_test)
