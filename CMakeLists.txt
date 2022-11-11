cmake_minimum_required(VERSION 3.10)
project(protocol C CXX)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

add_link_options(-static)

add_subdirectory(pybind11 REQUIRED)
add_subdirectory(serial)

if(WIN32)
    target_compile_definitions(serial PUBLIC UNICODE _UNICODE)
endif()

# a tool for generate crc table
add_executable(crc_gen crc_gen.c)

# python wrapper
add_subdirectory(fmt)
pybind11_add_module(protocol_lite python_wrapper.cpp protocol_lite.c protocol_lite.h)
target_link_libraries(protocol_lite PRIVATE fmt::fmt)

# cpp demo
add_executable(cpp_demo cpp_demo.cpp protocol_lite.c protocol_lite.h)
add_dependencies(cpp_demo serial)
target_link_libraries(cpp_demo PRIVATE serial fmt::fmt)
