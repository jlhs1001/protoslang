#ifndef PROTOSLANG_VM_H
#define PROTOSLANG_VM_H

#include "module.h"
#include "value.h"

// The maximum number of values that the VM can store on the stack.
// For now, we shall allocate a fixed amount of memory for the stack.
// However, future implementations may use a dynamic array to store values.
#define STACK_MAX 256

typedef struct {
    // The module that the VM will execute.
    Module* module;

    // The instruction pointer, which points to the next instruction to execute.
    uint8_t* ip;

    // The stack that the VM will use to store values.
    Value stack[256];

    // A pointer to the top of the stack.
    Value* stack_top;
} VM;

typedef enum {
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR
} InterpretResult;

// Initialize a VM.
void initialize_vm();

// Free the memory used by a VM.
void free_vm();

// Interpret a module.
InterpretResult interpret(const char *source);

// push a value onto the stack
void push(Value value);

// pop a value from the stack
Value pop();

#endif //PROTOSLANG_VM_H
