cmake_minimum_required(VERSION 3.10)
project(protocol C)

set(CMAKE_C_STANDARD 99)
set(POSITIVE_EXAMPLES_FILE "${CMAKE_CURRENT_SOURCE_DIR}/positive_example.txt")
set(NEGATIVE_EXAMPLES_FILE "${CMAKE_CURRENT_SOURCE_DIR}/negative_example.txt")

# protocol_lite
add_library(protocol protocol.c protocol.h)

add_executable(protocol_test_executable protocol_test.c)
target_link_libraries(protocol_test_executable protocol)

enable_testing()
add_test(protocol_usage_test protocol_test_executable)
set_tests_properties(protocol_usage_test PROPERTIES WILL_FAIL TRUE)
add_test(protocol_support_test protocol_test_executable 0 "${POSITIVE_EXAMPLES_FILE}")
add_test(protocol_pack_test protocol_test_executable 1 "${POSITIVE_EXAMPLES_FILE}")
add_test(protocol_unpack_test protocol_test_executable 2 "${POSITIVE_EXAMPLES_FILE}")
add_test(protocol_error_check_test protocol_test_executable 3 "${NEGATIVE_EXAMPLES_FILE}")
