// TODO: Break this file up into smaller files

#include <stdio.h>
#include <string.h>

#include "common.h"
#include "lexer.h"

typedef struct {
    const char *start;
    const char *current;
    int line;
} Lexer;

Lexer lexer;

void initialize_lexer(const char *source) {
    // Set the start and current pointers to the beginning of the source
    lexer.start = source;
    lexer.current = source;
    lexer.line = 1;
}

static bool is_alpha(char c) {
    // TODO: perhaps this function would be better named is_valid_identifier
    return (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z') ||
           c == '_';
}

static bool is_digit(char c) {
    return c >= '0' && c <= '9';
}

static bool is_at_end() {
    return *lexer.current == '\0';
}

static char advance() {
    lexer.current++;
    return lexer.current[-1];
}

static bool match(char expected) {
    // expect the current character to be the specified character
    if (is_at_end() || *lexer.current != expected) return false;

    lexer.current++;
    return true;
}

static char peek() {
    return *lexer.current;
}

static char peek_next() {
    if (is_at_end()) return '\0';
    return lexer.current[1];
}

static Token make_token(TokenType type) {
    // TODO: make docs for this function
    Token token;
    token.type = type;
    token.start = lexer.start;
    token.length = (int) (lexer.current - lexer.start);
    token.line = lexer.line;
    return token;
}

static Token error_token(const char *message) {
    Token token;
    token.type = TK_ERROR;
    token.start = message;
    token.length = (int) strlen(message);
    token.line = lexer.line;
    return token;
}

static void skip_whitespace() {
    for (;;) {
        char c = *lexer.current;
        switch (c) {
            case ' ':
            case '\r':
            case '\t':
                advance();
                break;
            case '\n':
                lexer.line++;
                advance();
                break;
            case '/':
                if (peek_next() == '/') {
                    // a comment goes until the end of the line
                    while (peek() != '\n' && !is_at_end()) advance();
                } else {
                    return;
                }
                break;
            default:
                return;
        }
    }
}

static TokenType check_keyword(
        int start,
        int length,
        const char *rest,
        TokenType type
) {
    if (lexer.current - lexer.start == start + length &&
        memcmp(lexer.start + start, rest, length) == 0) {
        return type;
    }

    return TK_IDENTIFIER;
}

static TokenType identifier_type() {
    // This is a bit unorthodox in manner of appearance, bit it's a simple DFA
    // that determines the type of identifier present.

    // check if the identifier is a keyword
    switch (lexer.start[0]) {
        case 'a': return check_keyword(1, 2, "nd", TK_AND);
        case 'c': return check_keyword(1, 4, "lass", TK_CLASS);
        case 'e': return check_keyword(1, 3, "lse", TK_ELSE);
        case 'f':
            if (lexer.current - lexer.start > 1) {
                switch (lexer.start[1]) {
                    case 'n': return TK_FN;
                    case 'o': return check_keyword(2, 1, "r", TK_FOR);
                    case 'a': return check_keyword(2, 3, "lse", TK_FALSE);
                }
            }
            break;
        case 'i': return check_keyword(1, 1, "f", TK_IF);
        case 'l': return check_keyword(1, 2, "et", TK_LET);
        case 'n': return check_keyword(1, 3, "ull", TK_NIL);
        case 'o': return check_keyword(1, 1, "r", TK_OR);
        case 'p': return check_keyword(1, 6, "rintln", TK_PRINTLN);
        case 'r': return check_keyword(1, 5, "eturn", TK_RETURN);
        case 's':
            if (lexer.current - lexer.start > 1) {
                switch (lexer.start[1]) {
                    case 'e': return check_keyword(2, 2, "lf", TK_SELF);
                    case 'u': return check_keyword(2, 3, "per", TK_SUPER);
//                    case 't': return check_keyword(2, 4, "ring", TK_TYPE_STRING);
                }
            }
            break;
        case 't':
            return check_keyword(1, 3, "rue", TK_TRUE);
        case 'w': return check_keyword(1, 4, "hile", TK_WHILE);
    }

    return TK_IDENTIFIER;
}

static Token identifier() {
    // lex an identifier from the source code,
    // and return a token representing the identifier.

    while (is_alpha(peek()) || is_digit(peek())) advance();

    return make_token(identifier_type());
}

static Token number() {
    // lex a numeric literal from the source code,
    // and return a token representing the literal.

    while (is_digit(peek())) advance();

    // look for a decimal point
    if (peek() == '.' && is_digit(peek_next())) {
        // consume the '.'
        advance();

        while (is_digit(peek())) advance();
    }

    // notably, the lexeme is not yet converted to a double
    return make_token(TK_NUMBER);
}

static Token string() {
    while (peek() != '"' && !is_at_end()) {
        if (peek() == '\n') lexer.line++;
        advance();
    }

    if (is_at_end()) return error_token("Unterminated string.");

    // consume the closing "
    advance();
    return make_token(TK_STRING);
}

Token lex_token() {
    // skip a leading whitespace
    skip_whitespace();
    lexer.start = lexer.current;
    if (is_at_end()) return make_token(TK_EOF);

    char c = advance();

    // lex identifiers
    if (is_alpha(c)) return identifier();

    // lex numeric literals
    if (is_digit(c)) return number();

    switch (c) {
        // lex single character tokens
        case '(':
            return make_token(TK_LPAREN);
        case ')':
            return make_token(TK_RPAREN);
        case '{':
            return make_token(TK_LBRACE);
        case '}':
            return make_token(TK_RBRACE);
        case '[':
            return make_token(TK_LBRACKET);
        case ']':
            return make_token(TK_RBRACKET);
        case ';':
            return make_token(TK_SEMICOLON);
        case ',':
            return make_token(TK_COMMA);
        case '.':
            return make_token(TK_DOT);
        case '-':
            return make_token(TK_MINUS);
        case '+':
            return make_token(TK_PLUS);
        case '/':
            return make_token(TK_SLASH);
        case '*':
            return make_token(TK_STAR);

            // lex multi-character tokens
        case '!':
            return make_token(
                    match('=') ? TK_BANG_EQUAL : TK_BANG
            );
        case '=':
            return make_token(
                    match('=') ? TK_EQUAL_EQUAL : TK_EQUAL
            );
        case '<':
            return make_token(
                    match('=') ? TK_LESS_EQUAL : TK_LESS
            );
        case '>':
            return make_token(
                    match('=') ? TK_GREATER_EQUAL : TK_GREATER
            );

            // lex literals
        case '"':
            return string();
    }

    return error_token("Unexpected character.");
}