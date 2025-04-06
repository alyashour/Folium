cmake_minimum_required(VERSION 3.11)
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

## VERSIONING ##

# Get Git commit hash for build id
find_package(Git QUIET)
if (GIT_FOUND)
    execute_process(
        COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD 
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_HASH 
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
else()
    set(GIT_HASH "unknown")
endif()

# Generate build date
string(TIMESTAMP BUILD_DATE "%Y-%m-%d" UTC)

# Configure
configure_file(
    ${CMAKE_SOURCE_DIR}/src/version.h.in 
    ${CMAKE_BINARY_DIR}/generated/version.h 
    @ONLY
)

# Include the generated dir
include_directories(${CMAKE_BINARY_DIR}/generated)

## INSTALL PACKAGES ##

# Google Test
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/52204f78f94d7512df1f0f3bea1d47437a2c3a58.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Cpp-httplib
FetchContent_Declare(
  httplib
  GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
  GIT_TAG        v0.20.0
)

# Nlohmann/json
FetchContent_Declare(
  json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG        v3.11.3
)

# Make the libraries available
FetchContent_MakeAvailable(googletest httplib json)

## SET UP EXECUTABLE ##

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

target_link_libraries(folium-core PUBLIC nlohmann_json::nlohmann_json)
target_link_libraries(folium-core PUBLIC httplib)

# Add the executable
add_executable(folium-server src/main.cc)
target_link_libraries(folium-server PRIVATE folium-core)


# Force a rebuild on new version
add_custom_target(version-info DEPENDS ${CMAKE_BINARY_DIR}/generated/version.h)
add_dependencies(folium-server version-info)

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