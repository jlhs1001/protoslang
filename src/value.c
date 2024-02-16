#include <stdio.h>

#include "memory.h"
#include "value.h"

// Initialize a value array
void initialize_value_array(ValueArray* array) {
    // Set the capacity and count to 0
    array->capacity = 0;
    array->count = 0;
    array->values = NULL;
}

// Write a value to a value array
void write_value_array(ValueArray* array, Value value) {
    // Calculate new capacity and reallocate if necessary
    if (array->capacity < array->count + 1) {
        int old_capacity = array->capacity;
        array->capacity = GROW_CAPACITY(old_capacity);
        array->values = GROW_ARRAY(array->values, Value, old_capacity, array->capacity);
    }

    // Write the value to the array
    array->values[array->count] = value;
    array->count++;
}

// Free the memory used by a value array
void free_value_array(ValueArray* array) {
    // Free the array of values
    FREE_ARRAY(Value, array->values, array->capacity);
    // Reset the capacity and count
    initialize_value_array(array);
}
