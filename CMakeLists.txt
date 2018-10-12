cmake_minimum_required(VERSION 3.12)
project(algo C)

set(CMAKE_C_STANDARD 99)

add_executable(algo main.c node.c node.h test.c test.h bin_io.c bin_io.h adhuff_compress.c adhuff_compress.h adhuff_decompress.c adhuff_decompress.h)