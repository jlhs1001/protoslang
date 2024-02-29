#ifndef PROTOSLANG_OBJECT_H
#define PROTOSLANG_OBJECT_H

#include "common.h"
#include "value.h"

#define OBJ_TYPE(value) (AS_OBJ(value)->type)
// this macro facilitates safe casting of an object to a string
#define IS_STRING(value) is_obj_type(value, OBJ_STRING)
#define IS_LIST(value) is_obj_type(value, OBJ_LIST)

#define AS_STRING(value) ((ObjString*)AS_OBJ(value))
#define AS_CSTRING(value) (((ObjString*)AS_OBJ(value))->chars)
#define AS_LIST(value) ((ObjList*)AS_OBJ(value))

typedef enum {
    OBJ_STRING,
    OBJ_LIST,
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

// The implementation of lists will be as follows:
// - A list is an object with a type of OBJ_LIST.
// - It contains a count of the number of elements in the list.
// - It contains a pointer to an array of Value objects.
// - The array of Value objects will be dynamically allocated.
// - The array of Value objects will be reallocated as the list grows.
// - The array of Value objects will be freed when the list is freed.

typedef struct {
    Obj obj;
    int count;
    int capacity;
    Value* items;
} ObjList;

ObjString *take_string(char *chars, int length);
ObjString *copy_string(const char *chars, int length);
void print_object(Value value);
void print_list(ObjList* list);

// list operations
ObjList *allocate_list();
void append_to_list(ObjList *list, Value value);
void store_list(ObjList *list, int index, Value value);
Value read_list(ObjList *list, int index);
void delete_from_list(ObjList *list, int index);
bool is_valid_index(ObjList *list, int index);

static inline bool is_obj_type(Value value, ObjType type) {
    return IS_OBJ(value) && AS_OBJ(value)->type == type;
}

#endif //PROTOSLANG_OBJECT_H
