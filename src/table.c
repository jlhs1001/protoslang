#include <stdlib.h>
#include <string.h>

#include "memory.h"
#include "object.h"
#include "table.h"
#include "value.h"

#define TABLE_MAX_LOAD 0.75

void initialize_table(Table *table) {
    table->count = 0;
    table->capacity = 0;
    table->entries = NULL;
}

void free_table(Table *table) {
    FREE_ARRAY(Entry, table->entries, table->capacity);
    initialize_table(table);
}

static Entry *find_entry(Entry *entries, int capacity, ObjString *key) {
    uint32_t index = key->hash % capacity;
    Entry *tombstone = NULL;

    for (;;) {
        Entry *entry = &entries[index];

        if (entry->key == NULL) {
            if (IS_NIL(entry->value)) {
                // empty entry
                return tombstone != NULL ? tombstone : entry;
            } else {
                // we found a tombstone
                if (tombstone == NULL) tombstone = entry;
            }
        } else if (entry->key == key) {
            // we found the key
            return entry;
        }

        index = (index + 1) % capacity;
    }
}

bool table_get(Table *table, ObjString *key, Value *value) {
    if (table->count == 0) return false;

    Entry *entry = find_entry(table->entries, table->capacity, key);
    if (entry->key == NULL) return false;

    *value = entry->value;
    return true;
}

static void adjust_capacity(Table *table, int capacity) {
    Entry *entries = ALLOCATE(Entry, capacity);

    for (int i = 0; i < capacity; i++) {
        entries[i].key = NULL;
        entries[i].value = NIL_VAL;
    }

    table->count = 0;
    for (int i = 0; i < table->capacity; i++) {
        Entry *entry = &table->entries[i];
        if (entry->key == NULL) {
            continue;
        }

        Entry *dest = find_entry(entries, capacity, entry->key);
        dest->key = entry->key;
        dest->value = entry->value;

        // when we grow the capacity, we may end up with fewer entries
        // in the resulting larger array because all the tombstones are removed.
        table->count++;
    }

    FREE_ARRAY(Entry, table->entries, table->capacity);
    table->entries = entries;
    table->capacity = capacity;
}

bool table_set(Table *table, ObjString *key, Value value) {
    // ensure the table has enough capacity to store the new entry
    if (table->count + 1 > table->capacity * TABLE_MAX_LOAD) {
        int capacity = GROW_CAPACITY(table->capacity);
        adjust_capacity(table, capacity);
    }

    Entry *entry = find_entry(table->entries, table->capacity, key);

    bool is_new_key = entry->key == NULL;
    if (is_new_key && IS_NIL(entry->value)) table->count++;

    entry->key = key;
    entry->value = value;

    return is_new_key;
}

void table_add_all(Table *from, Table *to) {
    for (int i = 0; i < from->capacity; i++) {
        Entry *entry = &from->entries[i];
        if (entry->key != NULL) {
            table_set(to, entry->key, entry->value);
        }
    }
}

ObjString *table_find_string(Table *table, const char *chars, int length, uint32_t hash) {
    if (table->count == 0) return NULL;

    uint32_t index = hash % table->capacity;

    for (;;) {
        Entry *entry = &table->entries[index];

        if (entry->key == NULL) {
            if (IS_NIL(entry->value)) {
                // empty entry
                return NULL;
            }
        } else if (entry->key->length == length && entry->key->hash == hash && memcmp(entry->key->chars, chars, length) == 0) {
            // we found the key
            return entry->key;
        }

        index = (index + 1) % table->capacity;
    }
}
