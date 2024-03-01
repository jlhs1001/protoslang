#include <stdio.h>
#include <string.h>

#include "memory.h"
#include "object.h"
#include "table.h"
#include "value.h"
#include "vm.h"

#define ALLOCATE_OBJ(type, objectType) \
    (type*)allocate_object(sizeof(type), objectType)

static Obj *allocate_object(size_t size, ObjType type) {
    // allocate a new object onto the heap
    Obj *object = (Obj*)reallocate(NULL, 0, size);
    object->type = type;

    // insert the new object into the linked list of objects
    object->next = vm.objects;
    vm.objects = object;

    // return the new object
    return object;
}

ObjFunction *new_function() {
    ObjFunction *function = ALLOCATE_OBJ(ObjFunction, OBJ_FUNCTION);
    function->arity = 0;
    function->name = NULL;
    initialize_module(&function->module);
    return function;
}

ObjRange *allocate_range(double start, double end) {
    ObjRange *range = ALLOCATE_OBJ(ObjRange, OBJ_RANGE);
    range->start = start;
    range->end = end;
    return range;
}

ObjList *allocate_list() {
    ObjList *list = ALLOCATE_OBJ(ObjList, OBJ_LIST);
    list->count = 0;
    list->capacity = 0;
    list->items = NULL;
    return list;
}

void append_to_list(ObjList *list, Value value) {
    // calculate new capacity and reallocate if necessary
    if (list->capacity < list->count + 1) {
        int old_capacity = list->capacity;
        list->capacity = GROW_CAPACITY(old_capacity);
        list->items = GROW_ARRAY(list->items, Value, old_capacity, list->capacity);
    }

    // append the value to the list
    list->items[list->count] = value;
    list->count++;
}

void store_list(ObjList *list, int index, Value value) {
    // store the value in the list
    list->items[index] = value;
}

Value read_list(ObjList *list, int index) {
    // read the value from the list
    return list->items[index];
}

void delete_from_list(ObjList *list, int index) {
    // delete the value from the list
    for (int i = index; i < list->count - 1; i++) {
        list->items[i] = list->items[i + 1];
    }

    list->items[list->count - 1] = NIL_VAL;
    list->count--;
}

bool is_valid_index(ObjList *list, int index) {
    // check if the index is valid
    return index >= 0 && index < list->count;
}

static ObjString *allocate_string(char *chars, int length, uint32_t hash) {
    ObjString *string = ALLOCATE_OBJ(ObjString, OBJ_STRING);
    string->length = length;
    string->chars = chars;
    string->hash = hash;
    table_set(&vm.strings, string, NIL_VAL);
    return string;
}

static uint32_t hash_string(const char *key, int length) {
    // FNV-la hash function
    uint32_t hash = 2166136261u;
    for (int i = 0; i < length; i++) {
        hash ^= (uint8_t)key[i];
        hash *= 16777619;
    }

    return hash;
}

ObjString *take_string(char *chars, int length) {
    // calculate the hash of the string
    uint32_t hash = hash_string(chars, length);
    ObjString *interned = table_find_string(&vm.strings, chars, length, hash);

    if (interned != NULL) {
        FREE_ARRAY(char, chars, length + 1);
        return interned;
    }

    // return the new string
    return allocate_string(chars, length, hash);
}

ObjString *copy_string(const char *chars, int length) {
    // calculate the hash of the string
    uint32_t hash = hash_string(chars, length);
    ObjString *interned = table_find_string(&vm.strings, chars, length, hash);

    if (interned != NULL) {
        return interned;
    }

    // allocate a new string onto the heap
    char *heap_chars = ALLOCATE(char, length + 1);
    memcpy(heap_chars, chars, length);

    // null-terminate the string
    heap_chars[length] = '\0';

    // return the new string
    return allocate_string(heap_chars, length, hash);
}

void print_object(Value value) {
    switch (OBJ_TYPE(value)) {
        case OBJ_FUNCTION:
            print_function(AS_FUNCTION(value));
            break;
        case OBJ_STRING:
            printf("%s", AS_CSTRING(value));
            break;
        case OBJ_LIST:
            print_list(AS_LIST(value));
            break;
        case OBJ_RANGE:
            print_range(AS_RANGE(value));
            break;
    }
}

void print_function(ObjFunction *function) {
    if (function->name == NULL) {
        printf("<script>");
        return;
    }

    printf("<fn %s>", function->name->chars);
}

void print_list(ObjList* list) {
    printf("[");
    for (int i = 0; i < list->count; i++) {
        print_value(list->items[i]); // Assume printValue is a function that can print Lox values correctly
        if (i < list->count - 1) {
            printf(", ");
        }
    }
    printf("]");
}

void print_range(ObjRange* range) {
    printf("%g..%g", range->start, range->end);
}
