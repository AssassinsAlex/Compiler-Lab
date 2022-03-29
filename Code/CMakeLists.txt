cmake_minimum_required(VERSION 3.21)
project(Code C)

set(CMAKE_C_STANDARD 99)
add_compile_options(-Wall -Werror -Wextra -Wpedantic  -Wno-unused-parameter -fno-asm)

add_executable(test_struct test_struct.c semantic.c semantic.h)