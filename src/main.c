#include <stdio.h>
#include "common.h"
#include "module.h"
#include "debug.h"
#include "vm.h"

int main() {
    initialize_vm();

    Module module;
    initialize_module(&module);

    int constant = add_constant(&module, 1.2);
    write_module(&module, OP_CONSTANT, 123);
    write_module(&module, constant, 123);

    constant = add_constant(&module, 3.4);
    write_module(&module, OP_CONSTANT, 123);
    write_module(&module, constant, 123);

    write_module(&module, OP_ADD, 123);

    constant = add_constant(&module, 5.6);
    write_module(&module, OP_CONSTANT, 123);
    write_module(&module, constant, 123);

    write_module(&module, OP_DIVIDE, 123);
    write_module(&module, OP_NEGATE, 123);

    write_module(&module, OP_RETURN, 123);

    interpret(&module);
    free_vm();
    free_module(&module);
    return 0;
}
