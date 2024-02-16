#ifndef PROTOSLANG_MODULE_H
#define PROTOSLANG_MODULE_H

#include "common.h"
#include "value.h"

// Each instruction has a single byte opcode.
// This value determines the kind of instruction
// that the VM should execute: add, sub, look up variable, etc.
typedef enum {
    OP_RETURN,
    OP_CONSTANT
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
    int* lines;
    // The array of values in the module.
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
