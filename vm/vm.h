#ifndef PROTOSLANG_VM_H
#define PROTOSLANG_VM_H

#include "module.h"

typedef struct {
    // The module that the VM will execute.
    Module* module;

    // The instruction pointer, which points to the next instruction to execute.
    uint8_t* ip;
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
InterpretResult interpret(Module* module);

#endif //PROTOSLANG_VM_H
