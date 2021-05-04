cmake_minimum_required(VERSION 3.17)
project(static_code_analyser C)

set(CMAKE_C_STANDARD 11)

add_executable(static_code_analyser
        main.c
        block.c block.h
        statement.c statement.h
        entity.c entity.h
        cvector.c cvector.h
        string_functions.c string_functions.h
        test.c test.h
        parser.c parser.h
        print.c print.h)

find_library(PCRE_LIBRARY pcre)
target_link_libraries(static_code_analyser PRIVATE ${PCRE_LIBRARY} m)
