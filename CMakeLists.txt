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
add_executable(ucra-cli
    src/cli/main.c
    src/world_wrapper.c
)

# Link libraries to the executable
target_link_libraries(ucra-cli
    world
    ucra
    vv-dsp
)

# Try to find ZSTD for optional compression support
find_package(ZSTD)
if(ZSTD_FOUND)
    target_compile_definitions(ucra-cli PRIVATE USE_ZSTD=1)
    target_include_directories(ucra-cli PRIVATE ${ZSTD_INCLUDE_DIRS})
    target_link_libraries(ucra-cli PRIVATE ${ZSTD_LIBRARIES})
    message(STATUS "ZSTD found: enabling compression support")
else()
    message(STATUS "ZSTD not found: compression support disabled")
endif()

# Enable CTest before adding tests anywhere
enable_testing()

# Include directories for the executable
target_include_directories(ucra-cli PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/third_party/world/src
    ${CMAKE_SOURCE_DIR}/third_party/ucra/include
    ${CMAKE_SOURCE_DIR}/third_party/vv-dsp/include
)

# Build worldcache helper library
add_library(worldcache SHARED
    src/worldcache/worldcache_format.c
    src/worldcache/worldcache_serialize.c
    src/worldcache/worldcache_manager.c
)
target_include_directories(worldcache PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(worldcache PRIVATE vv-dsp)

# If ZSTD is found, enable compression in worldcache library too
if(ZSTD_FOUND)
    target_compile_definitions(worldcache PRIVATE USE_ZSTD=1)
    if(TARGET ZSTD::ZSTD)
        message(STATUS "Linking worldcache with imported target ZSTD::ZSTD")
        target_link_libraries(worldcache PUBLIC ZSTD::ZSTD)
    elseif(DEFINED ZSTD_LIBRARIES AND ZSTD_LIBRARIES)
        message(STATUS "Linking worldcache with ZSTD_LIBRARIES=${ZSTD_LIBRARIES}")
        target_include_directories(worldcache PUBLIC ${ZSTD_INCLUDE_DIRS})
        target_link_libraries(worldcache PUBLIC ${ZSTD_LIBRARIES})
    else()
        message(STATUS "Linking worldcache with raw 'zstd' library name")
        target_link_libraries(worldcache PUBLIC zstd)
    endif()
endif()

# Note: don't force zstd into the small worldcache test to avoid link complications in CI.
# ucra-cli still gets zstd if found.

# Test working directory: always source dir for consistent relative paths
set(TEST_WD ${CMAKE_SOURCE_DIR})

# Small unit test for worldcache manager
add_executable(test_worldcache_manager src/worldcache/test_worldcache_manager.c)
target_link_libraries(test_worldcache_manager PRIVATE worldcache)
if(ZSTD_FOUND)
    if(TARGET ZSTD::ZSTD)
        target_link_libraries(test_worldcache_manager PRIVATE ZSTD::ZSTD)
    else()
        target_link_libraries(test_worldcache_manager PRIVATE ${ZSTD_LIBRARIES})
    endif()
endif()
add_test(NAME worldcache_manager_test COMMAND test_worldcache_manager)
set_tests_properties(worldcache_manager_test PROPERTIES WORKING_DIRECTORY ${TEST_WD})
if(WIN32)
    # Ensure worldcache.dll is discoverable
    set_tests_properties(worldcache_manager_test PROPERTIES ENVIRONMENT "PATH=$<TARGET_FILE_DIR:worldcache>;$ENV{PATH}")
endif()

# Unit test for header layout
add_executable(test_worldcache_header src/worldcache/test_worldcache_header.c)
target_link_libraries(test_worldcache_header PRIVATE worldcache)
add_test(NAME worldcache_header_test COMMAND test_worldcache_header)
set_tests_properties(worldcache_header_test PROPERTIES WORKING_DIRECTORY ${TEST_WD})
if(WIN32)
    set_tests_properties(worldcache_header_test PROPERTIES ENVIRONMENT "PATH=$<TARGET_FILE_DIR:worldcache>;$ENV{PATH}")
endif()

# Unit test for serialization round-trip
add_executable(test_worldcache_serialize src/worldcache/test_worldcache_serialize.c)
target_link_libraries(test_worldcache_serialize PRIVATE worldcache)
add_test(NAME worldcache_serialize_test COMMAND test_worldcache_serialize)
set_tests_properties(worldcache_serialize_test PROPERTIES WORKING_DIRECTORY ${TEST_WD})
if(WIN32)
    set_tests_properties(worldcache_serialize_test PROPERTIES ENVIRONMENT "PATH=$<TARGET_FILE_DIR:worldcache>;$ENV{PATH}")
endif()

# Enable testing
enable_testing()

# Add a basic CLI test - test help output
add_test(
    NAME basic_executable_test
    COMMAND ucra-cli --help
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

# Print project info
message(STATUS "Building ${PROJECT_NAME} version ${PROJECT_VERSION}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
