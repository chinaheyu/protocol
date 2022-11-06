cmake_minimum_required(VERSION 3.10)
project(protocol C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 11)

add_executable(test main.c protocol_lite.c protocol_lite.h)
add_executable(crc_gen crc_gen.c)