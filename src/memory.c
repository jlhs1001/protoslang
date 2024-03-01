#include <stdlib.h>

#include "memory.h"
#include "vm.h"

void* reallocate(void* previous, size_t old_size, size_t new_size) {
    // If the new size is 0, free the previous allocation and return NULL.
    if (new_size == 0) {
        free(previous);
        return NULL;
    }

    // Reallocate the previous allocation to the new size, exit on failure.
    void* result = realloc(previous, new_size);
    if (result == NULL) exit(1);
    return result;
}

static void free_object(Obj *object) {
    switch (object->type) {
        case OBJ_STRING: {
            ObjString *string = (ObjString*)object;
            FREE_ARRAY(char, string->chars, string->length + 1);
            FREE(ObjString, string);
            break;
        }
        case OBJ_LIST: {
            ObjList *list = (ObjList*)object;
            FREE_ARRAY(Value, list->items, list->capacity);
            FREE(ObjList, list);
            break;
        }
        case OBJ_RANGE: {
            ObjRange *range = (ObjRange*)object;
            FREE(ObjRange, range);
            break;
        }
        case OBJ_FUNCTION: {
            ObjFunction *function = (ObjFunction*)object;
            free_module(&function->module);
            FREE(ObjFunction, object);
            break;
        }
    }
}

void free_objects() {
    Obj *object = vm.objects;
    while (object != NULL) {
        Obj *next = object->next;
        free_object(object);
        object = next;
    }
}
