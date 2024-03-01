#ifndef PROTOSLANG_VM_H
#define PROTOSLANG_VM_H

#include "module.h"
#include "value.h"
#include "table.h"
#include "object.h"

// The maximum number of values that the VM can store on the stack.
// For now, we shall allocate a fixed amount of memory for the stack.
// However, future implementations may use a dynamic array to store values.
#define FRAMES_MAX 64
#define STACK_MAX (FRAMES_MAX * UINT8_COUNT)

typedef struct {
    ObjFunction *function;
    uint8_t *ip;
    Value *slots;
} CallFrame;

typedef struct {
    // The module that the VM will execute.
    Module* module;

    // The instruction pointer, which points to the next instruction to execute.
    uint8_t* ip;

    // The call stack that the VM will use to store call frames.
    CallFrame frames[FRAMES_MAX];

    // The number of frames in the call stack.
    int frame_count;

    // The stack that the VM will use to store values.
    Value stack[STACK_MAX];

    // A pointer to the top of the stack.
    Value* stack_top;

    // The table of globals that the VM will use to store global variables.
    Table globals;

    // The table of strings that the VM will use to store strings.
    Table strings;

    // The linked list of objects in the heap.
    Obj* objects;

    // A temporary register for storing values.
    Value reg_0;
} VM;

typedef enum {
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR
} InterpretResult;

extern VM vm;

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
