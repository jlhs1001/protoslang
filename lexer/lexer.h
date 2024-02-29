#ifndef PROTOSLANG_LEXER_H
#define PROTOSLANG_LEXER_H

typedef enum {
    // 1-char tokens
    TK_LPAREN, TK_RPAREN,
    TK_LBRACE, TK_RBRACE,
    TK_LBRACKET, TK_RBRACKET,
    TK_COMMA, TK_DOT,
    TK_MINUS, TK_PLUS,
    TK_SEMICOLON, TK_SLASH, TK_STAR,
    TK_BANG, TK_EQUAL, TK_GREATER, TK_LESS,

    // n-char tokens
    TK_BANG_EQUAL, TK_EQUAL_EQUAL,
    TK_GREATER_EQUAL, TK_LESS_EQUAL,
    TK_RANGE,

    // literals
    TK_IDENTIFIER, TK_STRING, TK_NUMBER,

    // keywords
    TK_AND, TK_CLASS, TK_ELSE, TK_FALSE,
    TK_FN, TK_FOR, TK_IF, TK_NIL, TK_OR,
    TK_PRINTLN, TK_RETURN, TK_SUPER, TK_SELF,
    TK_TRUE, TK_LET, TK_WHILE, TK_IN,

    // error and end of file
    TK_ERROR, TK_EOF
} TokenType;

typedef struct {
    // the type of the token
    TokenType type;

    // the start of the token in the source code
    const char* start;

    // the length of the token in the source code
    int length;

    // the line number of the token in the source code
    int line;
} Token;

// initialize the lexer with the source code
void initialize_lexer(const char* source);

// scan the next token from the source code and return a Token object
Token lex_token();

#endif //PROTOSLANG_LEXER_H
