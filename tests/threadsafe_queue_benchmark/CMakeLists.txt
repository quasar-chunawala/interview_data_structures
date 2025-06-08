cmake_minimum_required(VERSION 3.27)

# Project
project(threadsafe_queue_benchmark)

# Set the C++ language standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED 23)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fno-omit-frame-pointer")

# set include directories
set(INCLUDE_DIRECTORIES
    ${gtest_SOURCE_DIR}/include
    ../../include/threadsafe_queue/
)

# Add source files
set(SOURCE_FILES 
    threadsafe_queue_benchmark.cpp
)

# Set output directory for all binaries
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}) # For static libraries

message(STATUS "Building the threadsafe_queue_benchmark target in Debug mode...")

add_executable(threadsafe_queue_benchmark ${SOURCE_FILES})

target_include_directories(threadsafe_queue_benchmark PUBLIC ${INCLUDE_DIRECTORIES})

target_link_libraries(threadsafe_queue_benchmark benchmark::benchmark)