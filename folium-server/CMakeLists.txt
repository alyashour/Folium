set(CMAKE_BUILD_TYPE Debug)

# Set the project name
cmake_minimum_required(VERSION 3.10)
project(FoliumServer)

# Set the C++ standard to C++20
# DO NOT CHANGE
set(CMAKE_CXX_STANDARD 20)

# Specify where to find the header files
include_directories(include)

# Add the source files
add_executable(folium-server
    src/main.cpp
)