cmake_minimum_required(VERSION 3.16)

# Project configuration
project(badge C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Build target selection
option(BUILD_EMBEDDED "Build for embedded target (RP2350)" OFF)
option(BUILD_DESKTOP "Build for desktop emulator" ON)

# Common subdirectory
add_subdirectory(common)

# Target-specific builds
if(BUILD_EMBEDDED)
    # Import PICO SDK
    include(embedded/pico_sdk_import.cmake)
    pico_sdk_init()
    add_subdirectory(embedded)
endif()

if(BUILD_DESKTOP)
    add_subdirectory(desktop)
endif()

# Default to desktop build if no target specified
if(NOT BUILD_EMBEDDED AND NOT BUILD_DESKTOP)
    set(BUILD_DESKTOP ON)
    add_subdirectory(desktop)
endif() 