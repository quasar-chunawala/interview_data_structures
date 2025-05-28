cmake_minimum_required(VERSION 3.27)

# Project name
project(interview_data_structures)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable testing
enable_testing()

# Add GoogleTest to the project
add_subdirectory(ext/googletest)

# Global include directories for all test projects
include_directories(
    ${gtest_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include
)

# Add subdirectories for test projects
add_subdirectory(tests/unique_ptr_test)
add_subdirectory(tests/shared_ptr_test)
add_subdirectory(tests/vector_test)
add_subdirectory(tests/forward_list_test)
