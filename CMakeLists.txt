cmake_minimum_required(VERSION 3.26)
project(protoslang C)

set(CMAKE_C_STANDARD 11)

# Include directory for header files
include_directories(include)

# Specify the executable and its source files
add_executable(slang_prototype
        src/main.c
)
