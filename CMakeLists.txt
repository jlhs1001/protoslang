cmake_minimum_required(VERSION 3.26)
project(protoslang C)

set(CMAKE_C_STANDARD 11)

# Include directory for header files
include_directories(include)
include_directories(lexer)
include_directories(vm)

# Specify the executable and its source files
add_executable(slang_prototype
        src/main.c
        include/common.h
        include/module.h
        src/module.c
        include/memory.h
        src/memory.c
        include/debug.h
        src/debug.c
        include/value.h
        src/value.c
        vm/vm.h
        vm/vm.c
        include/compiler.h
        src/compiler.c
        lexer/lexer.c
        lexer/lexer.h
        include/object.h
        src/object.c
        include/table.h
        src/table.c
)
