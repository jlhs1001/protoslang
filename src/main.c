#include <stdio.h>
#include "common.h"
#include "module.h"
#include "debug.h"

int main() {
    Module module;
    initialize_module(&module);

    uint32_t constant = add_constant(&module, 1.2);
    write_module(&module, OP_CONSTANT, 123);
    write_module(&module, (uint8_t)constant, 123);

    write_module(&module, OP_RETURN, 123);

    disassemble_module(&module, "slang module");
    free_module(&module);
    return 0;
}
