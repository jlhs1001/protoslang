#ifndef PROTOSLANG_VM_H
#define PROTOSLANG_VM_H

#include "module.h"

typedef struct {
    // The module that the VM will execute.
    Module* module;
} VM;

// Initialize a VM.
void initialize_vm();

// Free the memory used by a VM.
void free_vm();

#endif //PROTOSLANG_VM_H
