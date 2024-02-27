#ifndef PROTOSLANG_TABLE_H
#define PROTOSLANG_TABLE_H

#include "common.h"
#include "value.h"

typedef struct {
    ObjString * key;
    Value value;
} Entry;

typedef struct {
    int count;
    int capacity;
    Entry *entries;
} Table;

void initialize_table(Table *table);
void free_table(Table *table);
bool table_set(Table *table, ObjString *key, Value value);

// used to get a value from the table
// important for things like object inheritance
void table_add_all(Table *from, Table *to);

#endif //PROTOSLANG_TABLE_H
