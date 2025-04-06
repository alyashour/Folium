cmake_minimum_required(VERSION 3.10)
project(FoliumServer VERSION 0.1.0 LANGUAGES CXX)

# Set default build type (if not already set)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

# Set the C++ standard to C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Google Test
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/52204f78f94d7512df1f0f3bea1d47437a2c3a58.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Source code lib
add_library(folium-core
    src/auth.cc
    src/core.cc
    src/data_access_layer.cc
    src/dispatcher.cc
    src/http_gateway.cc
    src/logger.cc
)

# Set include directories for the library
target_include_directories(folium-core PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src 
)

# Add the executable
add_executable(folium-server src/main.cc)
target_link_libraries(folium-server PRIVATE folium-core)

## TESTING ##
# Enable testing
enable_testing()

# Add the test executable
add_executable(tests
    tests/test_logger.cc
    tests/test_core.cc
)

# Link against your library and Google Test
target_link_libraries(tests PRIVATE folium-core gtest gtest_main)

# Add the test to CTest
add_test(NAME AllTests COMMAND tests)

# Installation rules
install(TARGETS folium-server DESTINATION bin)
install(TARGETS folium-core 
    EXPORT folium-core-targets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)
install(DIRECTORY include/ DESTINATION include)