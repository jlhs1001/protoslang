#include <stdio.h>

#include "common.h"
#include "compiler.h"
#include "lexer.h"

void compile(const char* source) {
    initialize_lexer(source);

    int line = -1;
    for (;;) {
        // scan the next token
        Token token = scan_token();

        // for now, simply print that token
        if (token.line != line) {
            printf("%4d ", token.line);
            line = token.line;
        } else {
            printf("   | ");
        }

        printf("%2d '%.*s'\n", token.type, token.length, token.start);

        // break on eof
        if (token.type == TOKEN_EOF) break;
    }
}