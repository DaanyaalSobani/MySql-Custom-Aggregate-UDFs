cmake_minimum_required(VERSION 3.10)

# Architecture options
option(BUILD_X86 "Build for x86_64 architecture" OFF)
option(BUILD_ARM "Build for ARM architecture" OFF)

# Detect host architecture
if(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "^(aarcharm64)$")
    set(HOST_IS_ARM TRUE)
    message(STATUS "Host architecture: ARM")
endif()

if(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "^(x86_64|AMD64)$")
    set(HOST_IS_X86 TRUE)
    message(STATUS "Host architecture: x86_64")
endif()

# Error check for conflicting options
if(BUILD_ARM AND BUILD_X86)
    message(FATAL_ERROR "Cannot build for both ARM and x86 simultaneously")
endif()

# Handle cross-compilation settings
if(BUILD_ARM AND NOT HOST_IS_ARM)
    set(CMAKE_SYSTEM_NAME Linux)
    set(CMAKE_SYSTEM_PROCESSOR aarch64)
    set(CMAKE_C_COMPILER aarch64-linux-gnu-gcc)
    set(CMAKE_CXX_COMPILER aarch64-linux-gnu-g++)
    message(STATUS "Cross-compiling for ARM")
endif()

if(BUILD_X86 AND NOT HOST_IS_X86)
    set(CMAKE_SYSTEM_NAME Linux)
    set(CMAKE_SYSTEM_PROCESSOR x86_64)
    set(CMAKE_C_COMPILER x86_64-linux-gnu-gcc)
    set(CMAKE_CXX_COMPILER x86_64-linux-gnu-g++)
    message(STATUS "Cross-compiling for x86_64")
endif()

if(NOT BUILD_ARM AND NOT BUILD_X86)
    message(STATUS "Using native compilation")
endif()

project(udf_product)

add_library(product SHARED src/product.cpp)

set_target_properties(product PROPERTIES
    PREFIX ""
    POSITION_INDEPENDENT_CODE ON
)
