cmake_minimum_required(VERSION 3.21)
project(Lab_compiler C)

set(CMAKE_C_STANDARD 99)

add_compile_options(-Werror -Wall -Wno-unused-function)

include_directories(${PROJECT_SOURCE_DIR}/Code)

if (CMAKE_SYSTEM_NAME MATCHES "Linux")
    set(CMAKE_C_FLAGS "-lfl -ly")
elseif (CMAKE_SYSTEM_NAME MATCHES "Windows")
    set(CMAKE_C_FLAGS "-LC:/GnuWin32/lib/")
endif(CMAKE_SYSTEM_NAME MATCHES "Linux")

add_custom_target(TEMP)

add_custom_command(TARGET TEMP
        PRE_BUILD
        COMMAND echo "FLEX" && cd ${PROJECT_SOURCE_DIR}/Code && flex lexical.l
        COMMAND echo "BISON" && cd ${PROJECT_SOURCE_DIR}/Code && bison -v -d syntax.y
        )


file(GLOB SRC_LIST ${PROJECT_SOURCE_DIR}/Code/*.c)
file(GLOB UNUSED ${PROJECT_SOURCE_DIR}/Code/lex.yy.c)

list(REMOVE_ITEM SRC_LIST ${UNUSED})
message("files:${SRC_LIST}")
message("Exclude:${UNUSED}")

add_executable(parse ${SRC_LIST})
