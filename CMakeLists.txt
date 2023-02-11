cmake_minimum_required(VERSION 3.10)
project(protocol VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")
if (MSVC)
    set(PROTOCOL_COMPILE_OPTIONS /W4 /WX)
else()
    set(PROTOCOL_COMPILE_OPTIONS -Wall -Wextra -Werror)
endif()

include(GNUInstallDirs)
include(PrintCmakeVariables)
print_cmake_variables()

if(NOT DEFINED CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

option(DEFAULT_SHARED_LIBS "Alias protocol to protocol_shared." OFF)
option(BUILD_PYTHON_BINDINGS "Build the python wrapper." ON)
option(INSTALL_PYTHON_BINDINGS "Install the python wrapper to site-packages/dist-packages." ON)
option(BUILD_TEST "Add test." OFF)

# protocol library
file(GLOB PROTOCOL_SOURCES src/*.c)
file(GLOB PROTOCOL_HEADERS include/*.h)

add_library(protocol_interface INTERFACE)
set_target_properties(protocol_interface PROPERTIES PUBLIC_HEADER "${PROTOCOL_HEADERS}")
target_include_directories(protocol_interface INTERFACE
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")

add_library(protocol_static STATIC ${PROTOCOL_SOURCES})
target_compile_options(protocol_static PRIVATE ${PROTOCOL_COMPILE_OPTIONS})
set_target_properties(protocol_static PROPERTIES OUTPUT_NAME protocol)
target_link_libraries(protocol_static PUBLIC protocol_interface)

add_library(protocol_shared SHARED ${PROTOCOL_SOURCES})
target_compile_options(protocol_shared PRIVATE ${PROTOCOL_COMPILE_OPTIONS})
set_target_properties(protocol_shared PROPERTIES OUTPUT_NAME protocol POSITION_INDEPENDENT_CODE ON)
target_link_libraries(protocol_shared PUBLIC protocol_interface)

if(DEFAULT_SHARED_LIBS)
    add_library(protocol ALIAS protocol_shared)
else()
    add_library(protocol ALIAS protocol_static)
endif()

# protocol test
if(BUILD_TEST)
    enable_testing()
    add_subdirectory(test)
endif()

# protocol wrapper
add_subdirectory(wrappers)

# install protocol
install(TARGETS protocol_static
        EXPORT protocol_targets
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/shared
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/static
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(TARGETS protocol_shared
        EXPORT protocol_targets
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/shared
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/static
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(TARGETS protocol_interface
        EXPORT protocol_targets
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/shared
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/static
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(EXPORT protocol_targets
        FILE protocolTargets.cmake
        NAMESPACE protocol::
        DESTINATION cmake)
if(NOT TARGET uninstall)
    configure_file(
            "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
            "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
            IMMEDIATE @ONLY)
    add_custom_target(uninstall
            COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
endif()

# generate package
include(CMakePackageConfigHelpers)
configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/protocolConfig.cmake.in
        "${CMAKE_CURRENT_BINARY_DIR}/protocolConfig.cmake"
        INSTALL_DESTINATION cmake)
write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/protocolConfigVersion.cmake
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY ExactVersion)
install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/protocolConfig.cmake"
        ${CMAKE_CURRENT_BINARY_DIR}/protocolConfigVersion.cmake
        DESTINATION cmake)
