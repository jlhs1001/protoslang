#ifndef PROTOSLANG_MODULE_H
#define PROTOSLANG_MODULE_H

#include "common.h"

// Each instruction has a single byte opcode.
// This value determines the kind of instruction
// that the VM should execute: add, sub, look up variable, etc.
typedef enum {
    OP_RETURN,
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
} Module;

// Initialize a module.
void initialize_module(Module* module);

// Write a byte to the end of a module.
void write_byte(Module* module, uint8_t byte);

#endif //PROTOSLANG_MODULE_H
