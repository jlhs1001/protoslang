#include <stdio.h>
#include "common.h"
#include "module.h"
#include "debug.h"

int main() {
    Module module;
    initialize_module(&module);

    write_module(&module, OP_RETURN);
    int constant = add_constant(&module, 1.2);

    write_module(&module, OP_CONSTANT);
    write_module(&module, (uint8_t)constant);

    disassemble_module(&module, "slang module");
    free_module(&module);
    return 0;
}
