#include <stdio.h>

#include "debug.h"

void disassemble_module(Module* module, const char* name) {
    printf("=== %s ===\n", name);

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

static int jump_instruction(const char* name, int sign, Module* module, int offset) {
    // Read in the two bytes of the jump offset.
    uint16_t jump = (uint16_t)(module->code[offset + 1] << 8);
    jump |= module->code[offset + 2];

    // Print the jump offset and the target of the jump.
    printf("%-16s %4d -> %d\n", name, offset, offset + 3 + sign * jump);

    // Return the offset of the next instruction in the module's array of instructions.
    return offset + 3;
}

int disassemble_instruction(Module* module, uint32_t offset) {
    printf("%04d ", offset);
    // Print the line number if the current instruction is on the same line as the previous instruction.
    if (offset > 0 && module->lines[offset] == module->lines[offset - 1]) {
        printf("   | ");
    } else {
        printf("%4d ", module->lines[offset]);
    }

    uint8_t instruction = module->code[offset];
    switch (instruction) {
        case OP_PRINTLN:
            return simple_instruction("println", (int)offset);
        case OP_JUMP:
            return jump_instruction("jmp", 1, module, (int)offset);
        case OP_JUMP_IF_FALSE:
            return jump_instruction("jif", 1, module, (int)offset);
        case OP_RETURN:
            return simple_instruction("ret", (int)offset);
        case OP_CONSTANT:
            return constant_instruction("imm", module, (int)offset);
        case OP_NIL:
            return simple_instruction("nil", (int)offset);
        case OP_TRUE:
            return simple_instruction("tru", (int)offset);
        case OP_FALSE:
            return simple_instruction("fal", (int)offset);
        case OP_POP:
            return simple_instruction("pop", (int)offset);
        case OP_GET_LOCAL:
            return constant_instruction("ldl", module, (int)offset);
        case OP_SET_LOCAL:
            return constant_instruction("stl", module, (int)offset);
        case OP_GET_GLOBAL:
            return constant_instruction("ldg", module, (int)offset);
        case OP_DEFINE_GLOBAL:
            return constant_instruction("stg", module, (int)offset);
        case OP_SET_GLOBAL:
            return constant_instruction("stg", module, (int)offset);
        case OP_EQUAL:
            return simple_instruction("equ", (int)offset);
        case OP_GREATER:
            return simple_instruction("grt", (int)offset);
        case OP_LESS:
            return simple_instruction("les", (int)offset);
        case OP_ADD:
            return simple_instruction("add", (int)offset);
        case OP_SUBTRACT:
            return simple_instruction("sub", (int)offset);
        case OP_MULTIPLY:
            return simple_instruction("mul", (int)offset);
        case OP_DIVIDE:
            return simple_instruction("div", (int)offset);
        case OP_NOT:
            return simple_instruction("not", (int)offset);
        case OP_NEGATE:
            return simple_instruction("neg", (int)offset);
        default:
            printf("Unknown opcode %d\n", instruction);
            return (int)offset + 1;
    }
}
