#include <stdio.h>
#include "common.h"
#include "module.h"
#include "debug.h"

int main() {
    Module module;
    initialize_module(&module);
    write_module(&module, OP_RETURN);
    disassemble_module(&module, "slang module");
    free_module(&module);
    return 0;
}
