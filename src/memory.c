#include <stdlib.h>

#include "memory.h"

void* reallocate(void* previous, size_t old_size, size_t new_size) {
    // If the new size is 0, free the previous allocation and return NULL.
    if (new_size == 0) {
        free(previous);
        return NULL;
    }

    // Reallocate the previous allocation to the new size, exit on failure.
    void* result = realloc(previous, new_size);
    if (result == NULL) exit(1);
    return result;
}