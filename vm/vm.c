#include <stdio.h>

#include "common.h"
#include "vm.h"

VM vm;

void initialize_vm() {
//    vm->module = module;
}

void free_vm() {
//    vm->module = NULL;
}

static InterpretResult run() {
#define READ_BYTE() (*vm.ip++)
#define READ_CONSTANT() (vm.module->constants.values[READ_BYTE()])

    for (;;) {
        uint8_t instruction;
        switch (instruction = READ_BYTE()) {
            case OP_CONSTANT: {
                Value constant = READ_CONSTANT();
                print_value(constant);
                printf("\n");
                break;
            }
            case OP_RETURN: {
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
