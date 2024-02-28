#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "compiler.h"
#include "lexer.h"

#ifdef DEBUG_PRINT_CODE
#include "debug.h"
#endif

typedef struct {
    Token current;
    Token previous;
    bool had_error;
    bool panic_mode;
} Parser;

typedef enum {
    PREC_NONE,
    PREC_ASSIGNMENT,  // =
    PREC_OR,          // or
    PREC_AND,         // and
    PREC_EQUALITY,    // == !=
    PREC_COMPARISON,  // < > <= >=
    PREC_TERM,        // + -
    PREC_FACTOR,      // * /
    PREC_UNARY,       // ! -
    PREC_CALL,        // . ()
    PREC_PRIMARY
} Precedence;

// a simple zero argument function pointer type for parsing
typedef void (*ParseFn)();

typedef struct {
    ParseFn prefix;
    ParseFn infix;
    Precedence precedence;
} ParseRule;

Parser parser;
Module *compiling_module;

static Module *current_module() {
    return compiling_module;
}

static void error_at(Token *token, const char *message) {
    if (parser.panic_mode) return;
    parser.panic_mode = true;
    fprintf(stderr, "[line %d] Error", token->line);

    if (token->type == TK_EOF) {
        fprintf(stderr, " at end");
    } else if (token->type == TK_ERROR) {
        // Nothing.
    } else {
        fprintf(stderr, " at '%.*s'", token->length, token->start);
    }

    fprintf(stderr, ": %s\n", message);
    parser.had_error = true;
}

static void error_at_current(const char *message) {
    error_at(&parser.current, message);
}

static void error(const char *message) {
    error_at(&parser.previous, message);
}

static void advance() {
    parser.previous = parser.current;

    for (;;) {
        parser.current = lex_token();
        if (parser.current.type != TK_ERROR) break;

        error_at_current(parser.current.start);
    }
}

static void consume(TokenType type, const char *message) {
    if (parser.current.type == type) {
        advance();
        return;
    }

    error_at_current(message);
}


static bool check(TokenType type) {
    return parser.current.type == type;
}

static bool match(TokenType type) {
    if (!check(type)) return false;

    advance();

    return true;
}

static void emit_byte(uint8_t byte) {
    write_module(current_module(), byte, parser.previous.line);
}

static void emit_byte_pair(uint8_t byte1, uint8_t byte2) {
    emit_byte(byte1);
    emit_byte(byte2);
}

static void emit_return() {
    emit_byte(OP_RETURN);
}

static void expression();
static void statement();
static void declaration();

static ParseRule *get_rule(TokenType type);

static void parse_precedence(Precedence precedence);

static uint8_t make_constant(Value value) {
    int constant = add_constant(current_module(), value);

    // TODO: dynamically resize the constant pool
    if (constant > UINT8_MAX) {
        error("Too many constants in one module.");
        return 0;
    }

    return (uint8_t) constant;
}

static void end_compiler() {
    emit_return();
#ifdef DEBUG_PRINT_CODE
    if (!parser.had_error) {
        disassemble_module(current_module(), "code");
    }
#endif
}

static void binary() {
    TokenType operator_type = parser.previous.type;
    ParseRule *rule = get_rule(operator_type);
    parse_precedence((Precedence) (rule->precedence + 1));

    switch (operator_type) {
        case TK_BANG_EQUAL:
            emit_byte_pair(OP_EQUAL, OP_NOT);
            break;
        case TK_EQUAL_EQUAL:
            emit_byte(OP_EQUAL);
            break;
        case TK_GREATER:
            emit_byte(OP_GREATER);
            break;
        case TK_GREATER_EQUAL:
            emit_byte_pair(OP_LESS, OP_NOT);
            break;
        case TK_LESS:
            emit_byte(OP_LESS);
            break;
        case TK_LESS_EQUAL:
            emit_byte_pair(OP_GREATER, OP_NOT);
            break;
        case TK_PLUS:
            emit_byte(OP_ADD);
            break;
        case TK_MINUS:
            emit_byte(OP_SUBTRACT);
            break;
        case TK_STAR:
            emit_byte(OP_MULTIPLY);
            break;
        case TK_SLASH:
            emit_byte(OP_DIVIDE);
            break;
        default:
            return; // unreachable
    }
}

static void literal() {
    switch (parser.previous.type) {
        case TK_FALSE:
            emit_byte(OP_FALSE);
            break;
        case TK_TRUE:
            emit_byte(OP_TRUE);
            break;
        case TK_NIL:
            emit_byte(OP_NIL);
            break;
        default:
            return; // unreachable
    }
}

static void expression() {
    parse_precedence(PREC_ASSIGNMENT);
}

static void parse_precedence(Precedence precedence) {
    advance();
    ParseFn prefix_rule = get_rule(parser.previous.type)->prefix;
    if (prefix_rule == NULL) {
        // TODO: Make this error message more informative.
        error("Expected expression.");
        return;
    }

    prefix_rule();

    while (precedence <= get_rule(parser.current.type)->precedence) {
        advance();
        ParseFn infix_rule = get_rule(parser.previous.type)->infix;
        infix_rule();
    }
}

static uint8_t identifier_constant(Token *name) {
    return make_constant(OBJ_VAL(copy_string(name->start, name->length)));
}

static uint8_t parse_variable(const char *error_message) {
    consume(TK_IDENTIFIER, error_message);
    return identifier_constant(&parser.previous);
}

static void define_variable(uint8_t global) {
    emit_byte_pair(OP_DEFINE_GLOBAL, global);
}

static void variable_declaration() {
    uint8_t global = parse_variable("Expected variable name.");

    if (match(TK_EQUAL)) {
        // variable assignment handling
        expression();
    } else {
        // variable initialization handling
        emit_byte(OP_NIL);
    }

    consume(TK_SEMICOLON, "Expected ';' after variable declaration.");

    define_variable(global);
}

// TODO: Further investigate using context to determine the end of a statement or expression,
//  as opposed to using a semicolon as a synchronizing token.
//// check if the current token is a synchronizing token
//static bool is_sync() {
//    switch (parser.current.type) {
//        case TK_CLASS:
//        case TK_FN:
//        case TK_LET:
//        case TK_IF:
//        case TK_WHILE:
//        case TK_PRINTLN:
//        case TK_RETURN:
//        case TK_EOF:
//            return true;
//        default:
//            return false;
//    }
//}
//
//static bool is_expr_stmt_op(TokenType type) {
//    switch (type) {
//        case TK_EQUAL:
//        case TK_PLUS:
//        case TK_MINUS:
//        case TK_STAR:
//        case TK_SLASH:
//        case TK_BANG_EQUAL:
//        case TK_EQUAL_EQUAL:
//        case TK_GREATER:
//        case TK_GREATER_EQUAL:
//        case TK_LESS:
//        case TK_LESS_EQUAL:
//            return true;
//        default:
//            return false;
//    }
//}

static void expression_statement() {
    expression();
    consume(TK_SEMICOLON, "Expected ';' after expression.");
    emit_byte(OP_POP);
}

static void print_statement() {
    // require a left parenthesis after the 'println' keyword
    if (check(TK_LPAREN)) {
        advance();

        // compile the expression
        expression();

        // require a right parenthesis after the expression
        consume(TK_RPAREN, "Expected ')' after expression.");
    } else {
        error("Expected '(' after 'println'.");
    }

    // require a semicolon after the expression
    consume(TK_SEMICOLON, "Expected ';' after expression.");

    // emit the instruction to print the value
    emit_byte(OP_PRINTLN);
}

static void synchronize() {
    parser.panic_mode = false;

    while (parser.current.type != TK_EOF) {
        // advance to the next synchronizing token
        if (parser.previous.type == TK_SEMICOLON) return;

        switch (parser.current.type) {
            case TK_CLASS:
            case TK_FN:
            case TK_LET:
            case TK_IF:
            case TK_WHILE:
            case TK_PRINTLN:
            case TK_RETURN:
                return;
            default:
                // Do nothing.
                ;
        }

        advance();
    }
}

static void statement() {
    if (match(TK_PRINTLN)) {
        print_statement();
    } else {
        expression_statement();
    }
}

static void declaration() {
    if (match(TK_LET)) {
        // variable declaration handling
        variable_declaration();
    } else {
        // statement handling
        statement();
    }

    if (parser.panic_mode) {
        synchronize();
    }
}

static void grouping() {
    expression();
    consume(TK_RPAREN, "Expected ')' after expression.");
}

static void emit_constant(Value value) {
    emit_byte_pair(OP_CONSTANT, make_constant(value));
}

static void number() {
    double value = strtod(parser.previous.start, NULL);
    emit_constant(NUMBER_VAL(value));
}

static void string() {
    // take the characters directly from the lexeme, and eliminate the surrounding quotes.
    emit_constant(OBJ_VAL(copy_string(
            parser.previous.start + 1,
            parser.previous.length - 2
    )));
}

static void named_variable(Token name) {
    uint8_t arg = identifier_constant(&name);
    emit_byte_pair(OP_GET_GLOBAL, arg);
}

static void variable() {
    named_variable(parser.previous);
}

static void unary() {
    TokenType operator_type = parser.previous.type;

    // compile the operand
    expression();

    // emit the operator instruction
    switch (operator_type) {
        case TK_BANG:
            emit_byte(OP_NOT);
            break;
        case TK_MINUS:
            emit_byte(OP_NEGATE);
            break;
        default:
            return; // unreachable
    }
}

ParseRule rules[] = {
        [TK_LPAREN]         = {grouping, NULL, PREC_NONE},
        [TK_RBRACE]         = {NULL, NULL, PREC_NONE},
        [TK_LBRACE]         = {NULL, NULL, PREC_NONE},
        [TK_RPAREN]         = {NULL, NULL, PREC_NONE},
        [TK_COMMA]          = {NULL, NULL, PREC_NONE},
        [TK_DOT]            = {NULL, NULL, PREC_NONE},
        [TK_MINUS]          = {unary, binary, PREC_TERM},
        [TK_PLUS]           = {NULL, binary, PREC_TERM},
        [TK_SEMICOLON]      = {NULL, NULL, PREC_NONE},
        [TK_SLASH]          = {NULL, binary, PREC_FACTOR},
        [TK_STAR]           = {NULL, binary, PREC_FACTOR},
        [TK_BANG]           = {unary, NULL, PREC_NONE},
        [TK_BANG_EQUAL]     = {NULL, binary, PREC_NONE},
        [TK_EQUAL]          = {NULL, NULL, PREC_NONE},
        [TK_EQUAL_EQUAL]    = {NULL, binary, PREC_EQUALITY},
        [TK_GREATER]        = {NULL, binary, PREC_COMPARISON},
        [TK_GREATER_EQUAL]  = {NULL, binary, PREC_COMPARISON},
        [TK_LESS]           = {NULL, binary, PREC_COMPARISON},
        [TK_LESS_EQUAL]     = {NULL, binary, PREC_COMPARISON},
        [TK_IDENTIFIER]     = {variable, NULL, PREC_NONE},
        [TK_STRING]         = {string, NULL, PREC_NONE},
        [TK_NUMBER]         = {number, NULL, PREC_NONE},
        [TK_AND]            = {NULL, NULL, PREC_NONE},
        [TK_CLASS]          = {NULL, NULL, PREC_NONE},
        [TK_ELSE]           = {NULL, NULL, PREC_NONE},
        [TK_FALSE]          = {literal, NULL, PREC_NONE},
        [TK_FN]             = {NULL, NULL, PREC_NONE},
        [TK_FOR]            = {NULL, NULL, PREC_NONE},
        [TK_IF]             = {NULL, NULL, PREC_NONE},
        [TK_NIL]            = {literal, NULL, PREC_NONE}, // TODO: Rename to TK_NULL
        [TK_OR]             = {NULL, NULL, PREC_NONE},
        [TK_PRINTLN]        = {NULL, NULL, PREC_NONE},
        [TK_RETURN]         = {NULL, NULL, PREC_NONE},
        [TK_SUPER]          = {NULL, NULL, PREC_NONE},
        [TK_SELF]           = {NULL, NULL, PREC_NONE},
        [TK_TRUE]           = {literal, NULL, PREC_NONE},
        [TK_LET]            = {NULL, NULL, PREC_NONE},
        [TK_WHILE]          = {NULL, NULL, PREC_NONE},
        [TK_ERROR]          = {NULL, NULL, PREC_NONE},
        [TK_EOF]            = {NULL, NULL, PREC_NONE}
};



static ParseRule *get_rule(TokenType type) {
    return &rules[type];
}

bool compile(const char *source, Module *module) {
    initialize_lexer(source);
    compiling_module = module;

    parser.had_error = false;
    parser.panic_mode = false;

    advance();

    while (!match(TK_EOF)) {
        declaration();
    }

    end_compiler();
    return !parser.had_error;
}