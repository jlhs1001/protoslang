//
// Created by Liam Seewald on 2/16/24.
//

#ifndef PROTOSLANG_VALUE_H
#define PROTOSLANG_VALUE_H

#include "common.h"

typedef enum {
    TYPE_BOOL,
    TYPE_NIL,
    TYPE_NUMBER,
} ValueType;

// Note that each value is currently 16 bytes...
// This is wildly inefficient, but it's a good starting point.
// Will be changed later.
typedef struct {
    ValueType type;
    union {
        bool boolean;
        double number;
    } as;
} Value;

#define IS_BOOL(value) ((value).type == TYPE_BOOL)
#define IS_NIL(value) ((value).type == TYPE_NIL)
#define IS_NUMBER(value) ((value).type == TYPE_NUMBER)

#define AS_BOOL(value) ((value).as.boolean)
#define AS_NUMBER(value) ((value).as.number)

#define BOOL_VAL(value) ((Value){TYPE_BOOL, {.boolean = value}})
#define NIL_VAL ((Value){TYPE_NIL, {.number = 0}})
#define NUMBER_VAL(value) ((Value){TYPE_NUMBER, {.number = value}})

typedef struct {
    int capacity;
    int count;
    Value *values;
} ValueArray;

bool values_equal(Value a, Value b);

void initialize_value_array(ValueArray *array);

void write_value_array(ValueArray *array, Value value);

void free_value_array(ValueArray *array);

// Print a value to the console.
void print_value(Value value);

#endif //PROTOSLANG_VALUE_H
