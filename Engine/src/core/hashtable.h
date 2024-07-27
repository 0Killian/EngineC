/**
 * @file hashtable.h
 * @author Killian Bellouard (killianbellouard@gmail.com)
 * @brief This files defines a hash table.
 * @version 0.1
 * @date 2024-06-27
 */

#pragma once

#include <common.h>

typedef struct hashtable {
    u32 element_size;
    u32 capacity;
    b8 pointers;
    void *data;
} hashtable;

API void hashtable_init(hashtable *hashtable, u32 element_size, u32 initial_capacity_count, b8 pointers);
API b8 hashtable_insert(hashtable *hashtable, const char *key, void *value);
API b8 hashtable_set(hashtable *hashtable, const char *key, void *value);
API void *hashtable_get(hashtable *hashtable, const char *key);
API void hashtable_destroy(hashtable *hashtable);
