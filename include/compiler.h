#ifndef PROTOSLANG_COMPILER_H
#define PROTOSLANG_COMPILER_H

#include "object.h"
#include "vm.h"

ObjFunction *compile(const char *source);

#endif //PROTOSLANG_COMPILER_H
