//
// Created by Liam Seewald on 2/16/24.
//

#ifndef PROTOSLANG_VALUE_H
#define PROTOSLANG_VALUE_H

#include "common.h"

typedef double Value;

typedef struct {
    int capacity;
    int count;
    Value* values;
} ValueArray;

void initialize_value_array(ValueArray* array);
void write_value_array(ValueArray* array, Value value);
void free_value_array(ValueArray* array);

// Print a value to the console.
void print_value(Value value);

#endif //PROTOSLANG_VALUE_H
