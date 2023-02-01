cmake_minimum_required(VERSION 3.10)
project(protocol C)

set(CMAKE_C_STANDARD 99)

# protocol library
file(GLOB PROTOCOL_SOURCES src/*.c)
add_library(protocol ${PROTOCOL_SOURCES})
target_include_directories(protocol PUBLIC include)

# protocol test
add_subdirectory(test)
enable_testing()
add_test(protocol_support_test protocol_test_executable 0 "${POSITIVE_EXAMPLES_FILE}")
add_test(protocol_pack_test protocol_test_executable 1 "${POSITIVE_EXAMPLES_FILE}")
add_test(protocol_unpack_test protocol_test_executable 2 "${POSITIVE_EXAMPLES_FILE}")
add_test(protocol_error_check_test protocol_test_executable 3 "${NEGATIVE_EXAMPLES_FILE}")
