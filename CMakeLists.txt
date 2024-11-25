cmake_minimum_required(VERSION 3.20)

option(BUILD_X86 "Build x86 version" OFF)
option(BUILD_ARM "Build ARM version" OFF)


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