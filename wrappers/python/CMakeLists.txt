cmake_minimum_required(VERSION 3.10)

project(py_protocol)

add_subdirectory(../../third_party/pybind11 third_party/pybind11 EXCLUDE_FROM_ALL)
add_subdirectory(../../third_party/fmt third_party/fmt EXCLUDE_FROM_ALL)

if (pybind11_FOUND)
    # Build python wrapper.
    pybind11_add_module(py_protocol py_protocol.cpp ../../src/protocol.c)
    target_include_directories(py_protocol PRIVATE ../../include)
    target_link_libraries(py_protocol PRIVATE fmt::fmt-header-only)

    # Find site-packages path.
    if(INSTALL_PYTHON_BINDINGS)
        find_package(Python COMPONENTS Interpreter)
        execute_process(COMMAND ${Python_EXECUTABLE} -c "from distutils import sysconfig; print(sysconfig.get_python_lib())"
                OUTPUT_VARIABLE Python_SITE_PACKAGES_PATH
                OUTPUT_STRIP_TRAILING_WHITESPACE
                ERROR_QUIET)
    else()
        set(Python_SITE_PACKAGES_PATH python_wrapper)
    endif()

    # Install python wrapper.
    install(TARGETS py_protocol
            LIBRARY DESTINATION ${Python_SITE_PACKAGES_PATH})

    # Install MinGW DLLs.
    if(MINGW)
        message(STATUS "Installing system-libraries: MinGW DLLs.")
        get_filename_component(MINGW_PATH ${CMAKE_CXX_COMPILER} PATH)
        file(GLOB MINGW_DLLS ${MINGW_PATH}/*.dll)
        install(FILES ${MINGW_DLLS} DESTINATION ${Python_SITE_PACKAGES_PATH})
    endif()
endif()
