#include <stdio.h>
#include <string.h>

#include "object.h"
#include "memory.h"
#include "value.h"

// Initialize a value array
void initialize_value_array(ValueArray *array) {
    // Set the capacity and count to 0
    array->capacity = 0;
    array->count = 0;
    array->values = NULL;
}

// Write a value to a value array
void write_value_array(ValueArray *array, Value value) {
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
void free_value_array(ValueArray *array) {
    // Free the array of values
    FREE_ARRAY(Value, array->values, array->capacity);
    // Reset the capacity and count
    initialize_value_array(array);
}

// Print a value to the console.
void print_value(Value value) {
    switch (value.type) {
        case TYPE_BOOL:
            printf(AS_BOOL(value) ? "true" : "false");
            break;
        case TYPE_NIL:
            printf("nil");
            break;
        case TYPE_NUMBER:
            printf("%g", AS_NUMBER(value));
            break;
        case VAL_OBJ:
            print_object(value);
            break;
    }
}

bool values_equal(Value a, Value b) {
    if (a.type != b.type) return false;
    switch (a.type) {
        case TYPE_BOOL:
            return AS_BOOL(a) == AS_BOOL(b);
        case TYPE_NIL:
            return true;
        case TYPE_NUMBER:
            return AS_NUMBER(a) == AS_NUMBER(b);
        case VAL_OBJ: {
            ObjString *a_str = AS_STRING(a);
            ObjString *b_str = AS_STRING(b);
            return a_str->length == b_str->length && memcmp(a_str->chars, b_str->chars, a_str->length) == 0;
        }
        default:
            return false; // Unreachable.
    }
}
