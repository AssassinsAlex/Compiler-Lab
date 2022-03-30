cmake_minimum_required(VERSION 3.21)
project(Code C)

set(CMAKE_C_STANDARD 99)
#add_compile_options(-Wall -Werror -Wextra -Wpedantic  -Wno-unused-parameter -fno-asm)
add_compile_options(-lfl -ly)

file(GLOB_RECURSE SRC_FILES ./Code/*.c)
file(GLOB_RECURSE SRC_HEADER ./Code/*.h)

add_executable(debug ${SRC_FILES} ${SRC_HEADER})