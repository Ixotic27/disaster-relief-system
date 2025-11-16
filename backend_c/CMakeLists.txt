cmake_minimum_required(VERSION 3.10)
project(DisasterReliefSystem C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Set output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Source files
set(SOURCES
    src/main.c
    src/fileio.c
    src/allocation.c
    src/graph.c
    src/hashmap.c
    src/heap.c
    src/report.c
)

# Create executable
add_executable(resq ${SOURCES})

# Compiler flags
if(MSVC)
    target_compile_options(resq PRIVATE /W4)
else()
    target_compile_options(resq PRIVATE -Wall -Wextra)
endif()

# Math library for Linux/Mac
if(UNIX)
    target_link_libraries(resq m)
endif()