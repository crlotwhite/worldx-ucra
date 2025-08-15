cmake_minimum_required(VERSION 3.16)

project(worldx-ucra
    VERSION 1.0.0
    DESCRIPTION "WORLD-based UTAU vocal synthesizer with UCRA API"
    LANGUAGES C CXX
)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Compiler flags for better performance and debugging
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -Wall -Wextra")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0 -Wall -Wextra")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -DNDEBUG")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -DNDEBUG")
endif()

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/src)

# Add third-party dependencies
add_subdirectory(third_party/world)
add_subdirectory(third_party/ucra)
add_subdirectory(third_party/vv-dsp)

# Create main executable target
add_executable(ucra-cli src/main.cpp)

# Link libraries to the executable
target_link_libraries(ucra-cli
    world
    ucra
    vv-dsp
)

# Include directories for the executable
target_include_directories(ucra-cli PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/third_party/world/src
    ${CMAKE_SOURCE_DIR}/third_party/ucra/include
    ${CMAKE_SOURCE_DIR}/third_party/vv-dsp/include
)

# Enable testing
enable_testing()

# Add a basic test
add_test(
    NAME basic_executable_test
    COMMAND ucra-cli
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

# Print project info
message(STATUS "Building ${PROJECT_NAME} version ${PROJECT_VERSION}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
