cmake_minimum_required(VERSION 3.12)
project(adhuff_test C)

set(CMAKE_C_STANDARD 99)


add_executable(adhuff_test test.c)
target_link_libraries(adhuff_test adhuff_lib)
