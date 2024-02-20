#include <stdio.h>

#include "common.h"
#include "debug.h"
#include "vm.h"

VM vm;

void reset_stack() {
    vm.stack_top = vm.stack;
}

void initialize_vm() {
//    vm->module = module;
    reset_stack();
}

void free_vm() {
//    vm->module = NULL;
}

void push(Value value) {
    // Set the stack top pointer to the new value, then increment the stack top pointer.
    *vm.stack_top = value;
    vm.stack_top++;
}

Value pop() {
    // Decrement the stack top pointer, then return the value that it points to.
    vm.stack_top--;
    return *vm.stack_top;
}

static InterpretResult run() {
#define READ_BYTE() (*vm.ip++)
#define READ_CONSTANT() (vm.module->constants.values[READ_BYTE()])
#define BINARY_OP(op) \
    do { \
        double b = pop(); \
        double a = pop(); \
        push(a op b); \
    } while (false)

    for (;;) {
#ifdef DEBUG_TRACE_EXECUTION
        // Print the stack.
        printf("          ");

        // Loop through each value in the stack and print it.
        for (Value* slot = vm.stack; slot < vm.stack_top; slot++) {
            printf("[ ");
            print_value(*slot);
            printf(" ]");
        }

        // Disassemble the current instruction.
        disassemble_instruction(vm.module, (int)(vm.ip - vm.module->code));
#endif

        uint8_t instruction;
        switch (instruction = READ_BYTE()) {
            case OP_CONSTANT: {
                // Get the constant value and push it onto the stack.
                Value constant = READ_CONSTANT();
                push(constant);
                break;
            }
            case OP_NEGATE:
                // Pop a value from the stack, negate it, then push the result back onto the stack.
                push(-pop());
                break;
            case OP_RETURN: {
                // Pop the value from the stack and print it.
                print_value(pop());
                printf("\n");
                return INTERPRET_OK;
            }
        }
    }

#undef READ_BYTE
#undef READ_CONSTANT
}

InterpretResult interpret(Module* module) {
    vm.module = module;
    vm.ip = vm.module->code;
    return run();
}
