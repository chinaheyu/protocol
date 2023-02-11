cmake_minimum_required(VERSION 3.10)

project(protocol_wrappers)

set(DEPENDENCIES protocol)

if (BUILD_PYTHON_BINDINGS)
    add_subdirectory(python)
endif()
