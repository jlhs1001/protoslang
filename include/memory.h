#ifndef PROTOSLANG_MEMORY_H
#define PROTOSLANG_MEMORY_H

#include "common.h"

// Grow the capacity of an array of elements of type T.
#define GROW_CAPACITY(capacity) \
    ((capacity) < 8 ? 8 : (capacity) * 2)

// Free an array of elements of type T.
#define FREE_ARRAY(type, pointer, old_capacity) \
    reallocate(pointer, sizeof(type) * (old_capacity), 0)

// Reallocate an array of elements of type T from old_capacity to new_capacity.
#define GROW_ARRAY(previous, type, old_capacity, new_capacity) \
    (type*)reallocate(previous, sizeof(type) * (old_capacity), sizeof(type) * (new_capacity))

void* reallocate(void* previous, size_t old_size, size_t new_size);

#endif //PROTOSLANG_MEMORY_H
