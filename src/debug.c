#include <stdio.h>

#include "debug.h"

void disassemble_module(Module* module, const char* name) {
    printf("== %s ==\n", name);

    for (uint32_t offset = 0; offset < module->count;) {
        offset = disassemble_instruction(module, offset);
    }
}

int disassemble_instruction(Module* module, uint32_t offset) {
    printf("%04d ", offset);

    uint8_t instruction = module->code[offset];
    switch (instruction) {
        case OP_RETURN:
            printf("ret\n");
            return (int)offset + 1;
        default:
            printf("Unknown opcode %d\n", instruction);
            return (int)offset + 1;
    }
}

static int simple_instruction(const char* name, int offset) {
    printf("%s\n", name);
    return offset + 1;
}