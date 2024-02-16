#ifndef PROTOSLANG_DEBUG_H
#define PROTOSLANG_DEBUG_H

#include "module.h"

// Disassemble the code in a module.
void disassemble_module(Module* module, const char* name);

// Disassemble a single instruction in a module.
int disassemble_instruction(Module* module, uint32_t offset);

#endif //PROTOSLANG_DEBUG_H
