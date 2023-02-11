cmake_minimum_required(VERSION 3.10)
project(protocol_test C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(POSITIVE_EXAMPLES_FILE "${CMAKE_CURRENT_LIST_DIR}/positive_example.txt")
set(NEGATIVE_EXAMPLES_FILE "${CMAKE_CURRENT_LIST_DIR}/negative_example.txt")

# build ctest executable
add_executable(protocol_test_executable protocol_test.c)
add_dependencies(protocol_test_executable protocol)
target_link_libraries(protocol_test_executable protocol)

# generate examples
if((NOT EXISTS "${CMAKE_CURRENT_LIST_DIR}/positive_example.txt") OR (NOT EXISTS "${CMAKE_CURRENT_LIST_DIR}/negative_example.txt"))
    find_package(Python COMPONENTS Interpreter)
    execute_process(COMMAND ${Python_EXECUTABLE} ${CMAKE_CURRENT_LIST_DIR}/generate_examples.py -c 10000 -d 512 WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})
endif()

# add test
get_target_property(PROTOCOL_TEST_EXECUTABLE_DIRECTORY protocol_test_executable RUNTIME_OUTPUT_DIRECTORY)
add_test(protocol_support_test ${PROTOCOL_TEST_EXECUTABLE_DIRECTORY}/protocol_test_executable 0 "${POSITIVE_EXAMPLES_FILE}")
add_test(protocol_pack_test ${PROTOCOL_TEST_EXECUTABLE_DIRECTORY}/protocol_test_executable 1 "${POSITIVE_EXAMPLES_FILE}")
add_test(protocol_unpack_test ${PROTOCOL_TEST_EXECUTABLE_DIRECTORY}/protocol_test_executable 2 "${POSITIVE_EXAMPLES_FILE}")
add_test(protocol_error_check_test ${PROTOCOL_TEST_EXECUTABLE_DIRECTORY}/protocol_test_executable 3 "${NEGATIVE_EXAMPLES_FILE}")
