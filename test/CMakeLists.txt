cmake_minimum_required(VERSION 3.10)
project(protocol_test C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(POSITIVE_EXAMPLES_FILE "${CMAKE_CURRENT_LIST_DIR}/positive_example.txt" PARENT_SCOPE)
set(NEGATIVE_EXAMPLES_FILE "${CMAKE_CURRENT_LIST_DIR}/negative_example.txt" PARENT_SCOPE)

# build ctest executable
add_executable(protocol_test_executable protocol_test.c)
add_dependencies(protocol_test_executable protocol)
target_link_libraries(protocol_test_executable protocol)

# generate examples
find_package(Python COMPONENTS Interpreter)
execute_process(COMMAND ${Python_EXECUTABLE} ${CMAKE_CURRENT_LIST_DIR}/generate_examples.py -c 100000 WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})