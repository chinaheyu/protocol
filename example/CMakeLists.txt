cmake_minimum_required(VERSION 3.10)
project(cpp_demo)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

add_link_options(-static)

find_package(pybind11 REQUIRED)
find_package(serial)

if(WIN32)
    target_compile_definitions(serial PUBLIC UNICODE _UNICODE)
endif()

# cpp demo
add_executable(cpp_demo cpp_demo.cpp protocol_lite.c protocol_lite.h)
add_dependencies(cpp_demo serial)
target_link_libraries(cpp_demo PRIVATE serial fmt::fmt)
