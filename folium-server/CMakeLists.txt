set(CMAKE_BUILD_TYPE Debug)

# Set the project name
cmake_minimum_required(VERSION 3.10)
project(FoliumServer)

# Set the C++ standard to C++20
# DO NOT CHANGE
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Google Test
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/52204f78f94d7512df1f0f3bea1d47437a2c3a58.zip
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Specify where to find the header files
include_directories(include)

# Add the source files
add_executable(folium-server
    src/main.cc
    src/auth.cc 
    src/core.cc 
    src/data_access_layer.cc 
    src/dispatcher.cc 
    src/http_gateway.cc 
    src/logger.cc 
)