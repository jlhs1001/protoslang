#ifndef PROTOSLANG_MODULE_H
#define PROTOSLANG_MODULE_H

#include "common.h"
#include "value.h"

// Each instruction has a single byte opcode.
// This value determines the kind of instruction
// that the VM should execute: add, sub, look up variable, etc.
typedef enum {
    OP_RETURN,
    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_NOT,
    OP_NEGATE,
    OP_PRINTLN,

    // variable operations
    OP_INCREMENT,

    // control flow
    OP_JUMP,
    OP_JUMP_IF_FALSE,
    OP_JUMP_IF_TRUE,
    OP_LOOP,

    // list operations
    OP_BUILD_LIST,
    OP_INDEX_LIST,
    OP_STORE_LIST,

    // range operations
    OP_BUILD_RANGE,
    OP_RANGE_START,
    OP_RANGE_END,
    OP_INCREMENT_RANGE,

    // duplicate the top value on the stack
    OP_DUPLICATE,

    // type casting
//    OP_TO_STRING,

    OP_CONSTANT,
    OP_NIL,
    OP_TRUE,
    OP_FALSE,
    OP_POP,
    OP_GET_LOCAL,
    OP_SET_LOCAL,
    OP_GET_GLOBAL,
    OP_DEFINE_GLOBAL,
    OP_SET_GLOBAL,
    OP_SET_N_LOCAL,
    OP_SET_N_GLOBAL,
    OP_EQUAL,
    OP_GREATER,
    OP_LESS,
    OP_LESS_EQUAL,
    // TODO: implement !=, <=, and >=
} OpCode;

// A module is a collection of instructions.
// The VM will execute these instructions in order.
typedef struct {
    // The number of instructions in the module.
    uint32_t count;
    // The capacity of the module's array of instructions.
    uint32_t capacity;
    // The array of instructions.
    uint8_t* code;
    // The line number of each instruction for debugging. Note that this is
    // not interleaved with each instruction because it would require a byte
    // per instruction, which would take up extra cache space and potentially
    // result in more cache misses. Instead, the line number is stored in a
    // separate array and only used when debugging.
    // TODO: This could be optimized by using a run-length encoding scheme.
    //  Compress this data by using the aforementioned scheme.
    int* lines;
    // The array of values in the module.
    // TODO: Add support for a LONG_CONSTANT instruction. The current implementation
    //  only supports 256 constants, which is not enough for a real program. However,
    //  most modules will not use more than 256 constants, so supporting both 8-bit addressed
    //  and 24-bit addressed constants will result in greater locality and thus greater performance
    //  in the typical scenario.
    ValueArray constants;
} Module;

// Initialize a module.
void initialize_module(Module* module);

// Write a byte to the end of a module.
void write_module(Module* module, uint8_t byte, int line);

// Add a constant value to a module.
uint32_t add_constant(Module* module, Value value);

// Free the memory used by a module.
void free_module(Module* module);

#endif //PROTOSLANG_MODULE_H
