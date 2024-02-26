#include <stdio.h>
#include <string.h>

#include "memory.h"
#include "object.h"
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

static ObjString *allocate_string(char *chars, int length) {
    ObjString *string = ALLOCATE_OBJ(ObjString, OBJ_STRING);
    string->length = length;
    string->chars = chars;
    return string;
}

ObjString *take_string(char *chars, int length) {
    return allocate_string(chars, length);
}

ObjString *copy_string(const char *chars, int length) {
    // allocate a new string onto the heap
    char *heap_chars = ALLOCATE(char, length + 1);
    memcpy(heap_chars, chars, length);

    // null-terminate the string
    heap_chars[length] = '\0';

    // return the new string
    return allocate_string(heap_chars, length);
}

void print_object(Value value) {
    switch (OBJ_TYPE(value)) {
        case OBJ_STRING:
            printf("%s", AS_CSTRING(value));
            break;
    }
}
