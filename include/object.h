#ifndef PROTOSLANG_OBJECT_H
#define PROTOSLANG_OBJECT_H

#include "common.h"
#include "value.h"

#define OBJ_TYPE(value) (AS_OBJ(value)->type)
// this macro facilitates safe casting of an object to a string
#define IS_STRING(value) is_obj_type(value, OBJ_STRING)

#define AS_STRING(value) ((ObjString*)AS_OBJ(value))
#define AS_CSTRING(value) (((ObjString*)AS_OBJ(value))->chars)

typedef enum {
    OBJ_STRING,
} ObjType;

struct Obj {
    ObjType type;
    // pointer to the next object in the linked list.
    // this is used to keep track of all objects in the heap.
    struct Obj *next;
};

struct ObjString {
    Obj obj;
    int length;
    char *chars;
    uint32_t hash;
};

ObjString *take_string(char *chars, int length);
ObjString *copy_string(const char *chars, int length);
void print_object(Value value);

static inline bool is_obj_type(Value value, ObjType type) {
    return IS_OBJ(value) && AS_OBJ(value)->type == type;
}

#endif //PROTOSLANG_OBJECT_H
