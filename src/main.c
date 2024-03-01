#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "module.h"
#include "debug.h"
#include "vm.h"

// TODO: Investigate error recovery strategies

#define MAX_LINE_LENGTH 1024

static void repl() {
    char line[MAX_LINE_LENGTH];
    for (;;) {
        printf("protoslang> ");
        if (!fgets(line, MAX_LINE_LENGTH, stdin)) {
            printf("\n");
            break;
        }

        interpret(line);
    }
}

static char *read_file(const char *path) {
    // open the file
    FILE *file = fopen(path, "rb");

    // check if the file was opened successfully
    if (file == NULL) {
        fprintf(stderr, "Could not open file \"%s\".\n", path);
        exit(74);
    }

    // seek to the end of the file to determine its size
    fseek(file, 0L, SEEK_END);
    size_t file_size = ftell(file);
    rewind(file);

    // allocate memory for the source code
    char *source = (char *)malloc(file_size + 1);
    if (source == NULL) {
        fprintf(stderr, "Not enough memory to read \"%s\".\n", path);
        exit(74);
    }

    // get the number of bytes read and check for errors
    size_t bytes_read = fread(source, sizeof(char), file_size, file);
    if (bytes_read < file_size) {
        fprintf(stderr, "Could not read file \"%s\".\n", path);
        exit(74);
    }

    // insert a null terminator at the end of the source code
    source[bytes_read] = '\0';

    // close the file
    fclose(file);

    return source;
}

static void run_file(const char *path) {
    // load the source code from the specified file
    char *source = read_file(path);

    // interpret the source code
    InterpretResult result = interpret(source);
    free(source);

    // check the result of the interpretation
    if (result == INTERPRET_COMPILE_ERROR) exit(65);
    if (result == INTERPRET_RUNTIME_ERROR) exit(70);
}

int main(int argc, const char* argv[]) {
    initialize_vm();

    if (argc == 1) {
        repl();
    } else if (argc == 2) {
        run_file(argv[1]);
    } else {
        fprintf(stderr, "Usage: protoslang [path]\n");
        exit(64);
    }

    free_vm();
    return 0;
}











/*
 * int: 0-255
 *  - value: 8
 *  - width: 8-bits
 * bool: TRUE | FALSE
 *  - value: TRUE
 *  - width: 8-bits
 *
 * [[00001000][00000001]001101010101010101001011111100000111100011001110001110010100110000111000101010100101011010010]
 */