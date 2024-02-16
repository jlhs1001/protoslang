//
// Created by Liam Seewald on 2/16/24.
//

#include "module.h"
#include "memory.h"

// Initialize a module.
void initialize_module(Module* module) {
    // Set the initial capacity of the module's array of instructions to 0.
    // To append a byte to the end of the module, the write_byte() function will be called.
    module->count = 0;
    module->capacity = 0;
    module->code = NULL;
}

// Free the memory used by a module.
void free_module(Module* module) {
    // Free the array of instructions.
    FREE_ARRAY(uint8_t, module->code, module->capacity);
    // Reset the module's fields to their initial values.
    initialize_module(module);
}

// Write a byte to the end of a module.
void write_byte(Module* module, uint8_t byte) {
    // If the module's array of instructions is full, reallocate the array to double its capacity.
    if (module->capacity < module->count + 1) {
        uint32_t old_capacity = module->capacity;
        module->capacity = GROW_CAPACITY(old_capacity);
        module->code = GROW_ARRAY(module->code, uint8_t, old_capacity, module->capacity);
    }

    // Append the byte to the end of the module.
    module->code[module->count] = byte;
    module->count++;
}
