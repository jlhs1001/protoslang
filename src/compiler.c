#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    PREC_SUBSCRIPT,   // []
    PREC_RANGE,       // ..
    PREC_CALL,        // . ()
    PREC_PRIMARY
} Precedence;

// a simple zero argument function pointer type for parsing
typedef void (*ParseFn)(bool can_assign);

typedef struct {
    ParseFn prefix;
    ParseFn infix;
    Precedence precedence;
} ParseRule;

typedef struct {
    Token name;
    int depth;
} Local;

typedef struct {
    Local locals[UINT8_COUNT];
    int local_count;
    int scope_depth;
} Compiler;

Parser parser;
Compiler *current = NULL;
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

static uint8_t make_constant(Value value) {
    int constant = add_constant(current_module(), value);

    // TODO: dynamically resize the constant pool
    if (constant > UINT8_MAX) {
        error("Too many constants in one module.");
        return 0;
    }

    return (uint8_t) constant;
}

static void emit_constant(Value value) {
    emit_byte_pair(OP_CONSTANT, make_constant(value));
}

static void emit_loop(int loop_start) {
    emit_byte(OP_LOOP);

    int offset = (int) current_module()->count - loop_start + 2;
    if (offset > UINT16_MAX) {
        error("Loop body too large.");
    }

    emit_byte((offset >> 8) & 0xff);
    emit_byte(offset & 0xff);
}

static int emit_jump(uint8_t instruction) {
    // emit specified jump instruction
    emit_byte(instruction);

    // set placeholder bytes for the jump offset
    emit_byte(0xff);
    emit_byte(0xff);

    // return the offset of the placeholder bytes
    return (int) current_module()->count - 2;
}

static void emit_return() {
    emit_byte(OP_RETURN);
}

static void expression();

static void statement();

static void declaration();

static ParseRule *get_rule(TokenType type);

static void parse_precedence(Precedence precedence);

static void end_compiler() {
    emit_return();
#ifdef DEBUG_PRINT_CODE
    if (!parser.had_error) {
        disassemble_module(current_module(), "code");
    }
#endif
}

static void begin_scope() {
    current->scope_depth++;
}

static void end_scope() {
    current->scope_depth--;

    // remove the local variables that are no longer in scope
    while (current->local_count > 0 &&
           current->locals[current->local_count - 1].depth > current->scope_depth) {
        // TODO: This can be further optimized through the use of an instruction
        //  that takes a number of locals to pop off the stack.
        emit_byte(OP_POP);
        current->local_count--;
    }
}

static void binary(bool can_assign) {
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
        case TK_RANGE:
            emit_byte(OP_BUILD_RANGE);
            break;
        default:
            return; // unreachable
    }
}

static void literal(bool can_assign) {
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

static void block() {
    while (!check(TK_RBRACE) && !check(TK_EOF)) {
        declaration();
    }

    consume(TK_RBRACE, "Expected '}' after block.");
}

static void parse_precedence(Precedence precedence) {
    advance();
    ParseFn prefix_rule = get_rule(parser.previous.type)->prefix;
    if (prefix_rule == NULL) {
        // TODO: Make this error message more informative.
        error("Expected expression.");
        return;
    }

    bool can_assign = precedence <= PREC_ASSIGNMENT;
    prefix_rule(can_assign);

    while (precedence <= get_rule(parser.current.type)->precedence) {
        advance();
        ParseFn infix_rule = get_rule(parser.previous.type)->infix;
        infix_rule(can_assign);
    }

    if (can_assign && match(TK_EQUAL)) {
        error("Invalid assignment target.");
    }
}

static uint8_t identifier_constant(Token *name) {
    return make_constant(OBJ_VAL(copy_string(name->start, name->length)));
}

static bool identifiers_equal(Token *a, Token *b) {
    if (a->length != b->length) return false;
    return memcmp(a->start, b->start, a->length) == 0;
}

static int resolve_local(Compiler *compiler, Token *name) {
    // find the local variable in the scope chain
    for (int i = compiler->local_count - 1; i >= 0; i--) {
        Local *local = &compiler->locals[i];
        if (identifiers_equal(name, &local->name)) {
            if (local->depth == -1) {
                error("Cannot read local variable in its own initializer.");
            }
            return i;
        }
    }

    // assumed to be a global variable.
    return -1;
}

static void add_local(Token name) {
    if (current->local_count == UINT8_COUNT) {
        // TODO: FIX FIX FIX FIX FIX FIX OH MY GOD FIX FIX FIX FIX FIX FIX
        error("Maximum number of local variables reached.");
        return;
    }

    Local *local = &current->locals[current->local_count++];
    local->name = name;
    local->depth = -1;
}

static void declare_variable() {
    // we are in global scope if the scope depth is zero.
    if (current->scope_depth == 0) return;

    Token *name = &parser.previous;
    for (int i = current->local_count - 1; i >= 0; i--) {
        Local *local = &current->locals[i];
        if (local->depth != -1 && local->depth < current->scope_depth) {
            break;
        }

        if (identifiers_equal(name, &local->name)) {
            // redeclaration of a variable in the same scope is disallowed.
            error("Variable with this name already declared in this scope.");
        }
    }

    add_local(*name);
}

static uint8_t parse_variable(const char *error_message) {
    consume(TK_IDENTIFIER, error_message);

    declare_variable();
    if (current->scope_depth > 0) return 0;

    return identifier_constant(&parser.previous);
}

static void mark_initialized() {
    current->locals[current->local_count - 1].depth = current->scope_depth;
}

static void define_variable(uint8_t global) {
    if (current->scope_depth > 0) {
        mark_initialized();
        return;
    }


    emit_byte_pair(OP_DEFINE_GLOBAL, global);
}

static void patch_jump(int offset) {
    // subtract two to account for the bytecode for the jump offset
    int jump = (int) current_module()->count - offset - 2;

    // TODO: upgrade compiler facilities to handle larger jumps
    if (jump > UINT16_MAX) {
        error("Too much code to jump over.");
    }

    // patch the jump offset
    current_module()->code[offset] = (jump >> 8) & 0xff;
    current_module()->code[offset + 1] = jump & 0xff;
}

static void and_(bool can_assign) {
    int end_jump = emit_jump(OP_JUMP_IF_FALSE);

    emit_byte(OP_POP);
    parse_precedence(PREC_AND);

    patch_jump(end_jump);
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

static void if_statement() {
    // parse expression
    expression();

    // require a left brace after the expression, but don't consume it.
    // it will be consumed later in the statement function call.
    if (!check(TK_LBRACE)) {
        error("Expected '{' after 'if' condition.");
    }

    int then_jump = emit_jump(OP_JUMP_IF_FALSE);

    // each statement is required to have zero stack effect
    emit_byte(OP_POP); // discard the condition value

    // parse the then clause
    statement();

    int else_jump = emit_jump(OP_JUMP);

    patch_jump(then_jump);

    // handle else clause if present
    if (match(TK_ELSE)) {
        if (!check(TK_LBRACE)) {
            error("Expected '{' after 'if' condition.");
        }
        statement();

        // patch the jump over the else clause
    }

    patch_jump(else_jump);
}

static void while_statement() {
    int loop_start = current_module()->count;
    // parse while condition
    expression();

    // require a left brace after the expression, but don't consume it.
    if (!check(TK_LBRACE)) {
        error("Expected '{' after 'while' condition.");
    }

    // emit a jump to the loop body
    int exit_jump = emit_jump(OP_JUMP_IF_FALSE);
    emit_byte(OP_POP);
    statement();

    emit_loop(loop_start);

    // patch the jump to the loop body
    patch_jump(exit_jump);

    // pop the condition value to maintain zero stack effect
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
    } else if (match(TK_IF)) {
        if_statement();
    } else if (match(TK_WHILE)) {
        while_statement();
    } else if (match(TK_LBRACE)) {
        begin_scope();
        block();
        end_scope();
    } else {
        expression_statement();
    }
}

static void list(bool can_assign) {
    int item_count = 0;
    if (!check(TK_RBRACKET)) {
        do {
            if (check(TK_RBRACKET)) {
                // trailing comma handling
                break;
            }

            parse_precedence(PREC_OR);

            if (item_count == UINT8_COUNT) {
                error("Cannot have more than 255 items in a list.");
            }

            item_count++;
        } while (match(TK_COMMA));
    }

    consume(TK_RBRACKET, "Expected ']' after list.");

    emit_byte(OP_BUILD_LIST);
    emit_byte(item_count);
}

static void integer(bool can_assign) {
    long value = strtol(parser.previous.start, NULL, 10);
    emit_constant(NUMBER_VAL(value));
}

//static void range(bool can_assign) {
//    // Expect a number for the start of the range
//    parse_precedence(PREC_OR);
//    consume(TK_RANGE, "Expected '..' after start of range.");
//    // Expect a number for the end of the range
//    parse_precedence(PREC_OR);
//
//    emit_byte(OP_BUILD_RANGE);
//}

static void subscript(bool can_assign) {
    parse_precedence(PREC_OR);
    consume(TK_RBRACKET, "Expected ']' after subscript.");

    if (can_assign && match(TK_EQUAL)) {
        expression();
        emit_byte(OP_STORE_LIST);
    } else {
        emit_byte(OP_INDEX_LIST);
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

static void grouping(bool can_assign) {
    expression();
    consume(TK_RPAREN, "Expected ')' after expression.");
}

static void initialize_compiler(Compiler *compiler) {
    compiler->local_count = 0;
    compiler->scope_depth = 0;
    current = compiler;
}

static void number(bool can_assign) {
    double value = strtod(parser.previous.start, NULL);
    emit_constant(NUMBER_VAL(value));
}

static void or_(bool can_assign) {
    int else_jump = emit_jump(OP_JUMP_IF_FALSE);
    int end_jump = emit_jump(OP_JUMP);

    patch_jump(else_jump);
    emit_byte(OP_POP);

    parse_precedence(PREC_OR);
    patch_jump(end_jump);
}

static void string(bool can_assign) {
    // take the characters directly from the lexeme, and eliminate the surrounding quotes.
    emit_constant(OBJ_VAL(copy_string(
            parser.previous.start + 1,
            parser.previous.length - 2
    )));
}

static void named_variable(Token name, bool can_assign) {
    uint8_t get_op, set_op;
    int arg = resolve_local(current, &name);

    // determine the appropriate get and set instructions
    // to use based on whether the variable is local or global.
    if (arg != -1) {
        get_op = OP_GET_LOCAL;
        set_op = OP_SET_LOCAL;
    } else {
        arg = identifier_constant(&name);
        get_op = OP_GET_GLOBAL;
        set_op = OP_SET_GLOBAL;
    }

    if (can_assign && match(TK_EQUAL)) {
        expression();
        emit_byte_pair(set_op, (uint8_t) arg);
    } else {
        emit_byte_pair(get_op, (uint8_t) arg);
    }
}

static void variable(bool can_assign) {
    named_variable(parser.previous, can_assign);
}

static void unary(bool can_assign) {
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
        [TK_LBRACKET]       = {list, subscript, PREC_SUBSCRIPT},
        [TK_RBRACKET]       = {NULL, NULL, PREC_NONE},
        [TK_COMMA]          = {NULL, NULL, PREC_NONE},
        [TK_RANGE]          = {NULL, binary, PREC_RANGE},
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
        [TK_AND]            = {NULL, and_, PREC_NONE},
        [TK_CLASS]          = {NULL, NULL, PREC_NONE},
        [TK_ELSE]           = {NULL, NULL, PREC_NONE},
        [TK_FALSE]          = {literal, NULL, PREC_NONE},
        [TK_FN]             = {NULL, NULL, PREC_NONE},
        [TK_FOR]            = {NULL, NULL, PREC_NONE},
        [TK_IF]             = {NULL, NULL, PREC_NONE},
        [TK_NIL]            = {literal, NULL, PREC_NONE}, // TODO: Rename to TK_NULL
        [TK_OR]             = {NULL, or_, PREC_NONE},
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

    // initialize the compiler
    Compiler compiler;
    initialize_compiler(&compiler);

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