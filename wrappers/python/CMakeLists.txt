cmake_minimum_required(VERSION 3.10)

project(py_protocol)

if (pybind11_FOUND)
    pybind11_add_module(py_protocol py_protocol.cpp)
    add_dependencies(py_protocol protocol)
    target_link_libraries(py_protocol PRIVATE protocol fmt::fmt)

    install(TARGETS py_protocol
            RUNTIME DESTINATION python_wrappers
            LIBRARY DESTINATION python_wrappers
            ARCHIVE DESTINATION python_wrappers)

    if(MINGW)
        message(STATUS "Installing system-libraries: MinGW DLLs.")
        get_filename_component(MINGW_PATH ${CMAKE_CXX_COMPILER} PATH)
        file(GLOB MINGW_DLLS ${MINGW_PATH}/*.dll)
        install(FILES ${MINGW_DLLS} DESTINATION python_wrappers)
    endif()
endif ()
