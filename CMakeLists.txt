cmake_minimum_required(VERSION 3.10)
project(protocol C CXX)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

add_link_options(-static)

add_subdirectory(fmt)
add_subdirectory(pybind11)

# protocol_lite
add_library(protocol_lite protocol_lite.c protocol_lite.h)

# python wrapper
if (pybind11_FOUND)
    pybind11_add_module(py_protocol python_wrapper.cpp)
    target_link_libraries(py_protocol PRIVATE fmt::fmt protocol_lite)
endif ()
