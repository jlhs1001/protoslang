#ifndef PROTOSLANG_COMPILER_H
#define PROTOSLANG_COMPILER_H

#include "object.h"
#include "vm.h"

bool compile(const char *source, Module *module);

#endif //PROTOSLANG_COMPILER_H
