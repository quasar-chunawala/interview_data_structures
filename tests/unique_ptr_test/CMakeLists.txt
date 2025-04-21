cmake_minimum_required(VERSION 3.25)

# Project
project(unique_ptr_test)

# Set the C++ language standard
set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED 26)

# Add googletest as a subproject
# add_subdirectory(../../ext/googletest)

# set include directories
set(INCLUDE_DIRECTORIES
    ../../ext/googletest/googletest/include
    ../../include/
)

# Add source files
set(SOURCE_FILES 
    unique_ptr_test.cpp
)
