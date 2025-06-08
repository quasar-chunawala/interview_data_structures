cmake_minimum_required(VERSION 3.27)

# Project name
project(interview_data_structures)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(STATUS "Platform: Linux")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    message(STATUS "Platform: Windows")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    message(STATUS "Platform: macOS")
elseif(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")    
    message(STATUS "Platform: FreeBSD")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Android")    
    message(STATUS "Platform: Android")    
else()
    message(STATUS "Platform: Unknown (${CMAKE_SYSTEM_NAME})")
endif()

# Print the compiler executable
message(STATUS "C++ Compiler: ${CMAKE_CXX_COMPILER}")

# Check the compiler vendor
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    message(STATUS "Compiler: GCC")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    message(STATUS "Compiler: Clang")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    message(STATUS "Compiler: Microsoft Visual C++")
else()
    message(STATUS "Compiler: Unknown (${CMAKE_CXX_COMPILER_ID})")
endif()

# Enable testing
enable_testing()

# Add GoogleTest to the project
add_subdirectory(ext/googletest)

# Add Google benchmark micro-benchmarking library
add_subdirectory(ext/benchmark)

# Global include directories for all test projects
include_directories(
    ${gtest_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include
)

# Add subdirectories for test projects
add_subdirectory(tests/unique_ptr_test)
add_subdirectory(tests/shared_ptr_test)
# add_subdirectory(tests/vector_test)
add_subdirectory(tests/forward_list_test)
add_subdirectory(tests/threadsafe_stack_test)
add_subdirectory(tests/threadsafe_queue_test)
add_subdirectory(tests/threadsafe_queue_benchmark)