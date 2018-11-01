cmake_minimum_required(VERSION 3.12)
project(adhuff_exe C)
add_subdirectory(test)

set(CMAKE_C_STANDARD 99)

add_library(adhuff_lib bin_io.c bin_io.h adhuff_compress.c adhuff_compress.h adhuff_decompress.c adhuff_decompress.h constants.h adhuff_common.h adhuff_common.c log.c log.h)

add_executable(adhuff_exe main.c)
target_link_libraries(adhuff_exe adhuff_lib)
