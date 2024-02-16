#include <stdio.h>

#include "debug.h"

void disassemble_module(Module* module, const char* name) {
    printf("== %s ==\n", name);

    for (uint32_t offset = 0; offset < module->count;) {
        offset = disassemble_instruction(module, offset);
    }
}

static int simple_instruction(const char* name, int offset) {
    // Print the name of the instruction.
    printf("%s\n", name);
    // Return the offset of the next instruction in the module's array of instructions.
    return offset + 1;
}

static int constant_instruction(const char* name, Module* module, int offset) {
    // Get the index of the constant value from the next byte in the module's array of instructions.
    uint8_t constant = module->code[offset + 1];

    // Print the constant value and its index in the array of values.
    printf("%-16s %4d '", name, constant);
    print_value(module->constants.values[constant]);
    printf("'\n");

    // Return the offset of the next instruction in the module's array of instructions.
    return offset + 2;
}

int disassemble_instruction(Module* module, uint32_t offset) {
    printf("%04d ", offset);

    uint8_t instruction = module->code[offset];
    switch (instruction) {
        case OP_RETURN:
            return simple_instruction("ret", (int)offset);
        case OP_CONSTANT:
            return constant_instruction("imm", module, (int)offset);
        default:
            printf("Unknown opcode %d\n", instruction);
            return (int)offset + 1;
    }
}
