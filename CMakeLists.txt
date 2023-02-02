cmake_minimum_required(VERSION 3.10)
project(protocol C CXX)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/install)

if(NOT DEFINED CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

if(NOT DEFINED BUILD_PYTHON_BINDINGS)
    set(BUILD_PYTHON_BINDINGS ON)
endif()

if(UNIX AND NOT APPLE)
    add_compile_options(-fPIC)
endif()

# protocol library
file(GLOB PROTOCOL_SOURCES src/*.c)
file(GLOB PROTOCOL_HEADERS include/*.h)
add_library(protocol ${PROTOCOL_SOURCES})
target_include_directories(protocol PUBLIC include)

# protocol test
add_subdirectory(test)
enable_testing()
add_test(protocol_support_test protocol_test_executable 0 "${POSITIVE_EXAMPLES_FILE}")
add_test(protocol_pack_test protocol_test_executable 1 "${POSITIVE_EXAMPLES_FILE}")
add_test(protocol_unpack_test protocol_test_executable 2 "${POSITIVE_EXAMPLES_FILE}")
add_test(protocol_error_check_test protocol_test_executable 3 "${NEGATIVE_EXAMPLES_FILE}")

# protocol wrapper
add_subdirectory(third_party/pybind11 EXCLUDE_FROM_ALL)
add_subdirectory(third_party/fmt EXCLUDE_FROM_ALL)
add_subdirectory(wrappers)

# install protocol
install(TARGETS protocol
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib/shared
        ARCHIVE DESTINATION lib/static)
install(FILES ${PROTOCOL_HEADERS} DESTINATION include)
install(FILES ${PROTOCOL_SOURCES} DESTINATION src)
