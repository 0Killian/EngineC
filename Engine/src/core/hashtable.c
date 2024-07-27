#include "hashtable.h"
#include "memory.h"
#include <string.h>

#define LOG_SCOPE "HASHTABLE"
#include "core/log.h"

// FNV-1a (https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function)
u64 hash(const char *key) {
    const u64 FNV_PRIME = 0x100000001b3;
    const u64 FNV_OFFSET_BASIS = 0xcbf29ce484222325;

    u64 hash = FNV_OFFSET_BASIS;
    for (u64 i = 0; key[i] != '\0'; i++) {
        hash ^= key[i];
        hash *= FNV_PRIME;
    }
    return hash;
}

typedef struct hashtable_entry_header {
    b8 present;
    char *key;
} hashtable_entry_header;

static b8 resize_hashtable(hashtable *table, u32 new_capacity) {
    hashtable copy = *table;

    table->data = mem_alloc(MEMORY_TAG_HASHTABLE, new_capacity * (table->element_size + sizeof(hashtable_entry_header)));
    mem_zero(table->data, new_capacity * (table->element_size + sizeof(hashtable_entry_header)));
    table->capacity = new_capacity;

    // Rehashing
    for (u32 i = 0; i < copy.capacity; i++) {
        hashtable_entry_header *header =
            (hashtable_entry_header *)(copy.data + (i * (table->element_size + sizeof(hashtable_entry_header))));

        if (header->present) {
            // Need to transfer it over
            u64 index = hash(header->key) % table->capacity;
            u64 offset = (index * (table->element_size + sizeof(hashtable_entry_header)));
            hashtable_entry_header *new_header = (hashtable_entry_header *)(table->data + offset);

            if (new_header->present) {
                LOG_WARN(
                    "Hash key collision while rehashing: %s and %s (%d entries)", new_header->key, header->key, table->capacity);
                mem_free(table->data);
                *table = copy;
                return FALSE;
            }

            new_header->present = TRUE;
            new_header->key = header->key;
            mem_copy(new_header + sizeof(hashtable_entry_header), header + sizeof(hashtable_entry_header), table->element_size);
        }
    }

    mem_free(copy.data);
    return TRUE;
}

API void hashtable_init(hashtable *hashtable, u32 element_size, u32 initial_capacity, b8 pointers) {
    hashtable->element_size = element_size;
    hashtable->capacity = initial_capacity;
    hashtable->pointers = pointers;
    hashtable->data = mem_alloc(MEMORY_TAG_HASHTABLE, initial_capacity * (element_size + sizeof(hashtable_entry_header)));
    mem_zero(hashtable->data, initial_capacity * (element_size + sizeof(hashtable_entry_header)));
}

API b8 hashtable_insert(hashtable *hashtable, const char *key, void *value) {
    u64 index = hash(key) % hashtable->capacity;
    u64 offset = (index * (hashtable->element_size + sizeof(hashtable_entry_header)));
    hashtable_entry_header *header = (hashtable_entry_header *)(hashtable->data + offset);

    if (header->present) {
        if (strcmp(header->key, key) != 0) {
            if (!resize_hashtable(hashtable, hashtable->capacity * 2)) {
                return FALSE;
            }
            return hashtable_insert(hashtable, key, value);
        }

        return FALSE;
    }

    header->present = TRUE;
    header->key = mem_alloc(MEMORY_TAG_HASHTABLE, strlen(key) + 1);
    mem_copy(header->key, key, strlen(key) + 1);

    if (hashtable->pointers) {
        *(void **)(header + sizeof(hashtable_entry_header)) = value;
    } else {
        mem_copy(header + sizeof(hashtable_entry_header), value, hashtable->element_size);
    }

    return TRUE;
}

API b8 hashtable_set(hashtable *hashtable, const char *key, void *value) {
    u64 index = hash(key) % hashtable->capacity;
    u64 offset = (index * (hashtable->element_size + sizeof(hashtable_entry_header)));
    hashtable_entry_header *header = (hashtable_entry_header *)(hashtable->data + offset);

    if (!header->present) {
        return FALSE;
    }

    if (strcmp(header->key, key) != 0) {
        LOG_ERROR("Hash key collision while setting: %s and %s (%d entries)\n This should never happen if hashtable_insert "
                  "verify collisions correctly",
                  header->key,
                  key,
                  hashtable->capacity);
        return TRUE;
    }

    if (value == NULL) {
        header->present = FALSE;
        mem_free(header->key);
        return TRUE;
    }

    if (hashtable->pointers) {
        *(void **)(header + sizeof(hashtable_entry_header)) = value;
    } else {
        mem_copy(header + sizeof(hashtable_entry_header), value, hashtable->element_size);
    }

    return TRUE;
}

API void *hashtable_get(hashtable *hashtable, const char *key) {
    u64 index = hash(key) % hashtable->capacity;
    u64 offset = (index * (hashtable->element_size + sizeof(hashtable_entry_header)));
    hashtable_entry_header *header = (hashtable_entry_header *)(hashtable->data + offset);

    if (!header->present) {
        return NULL;
    }

    if (strcmp(header->key, key) != 0) {
        LOG_WARN("Hash key collision: %s and %s (%d entries)", header->key, key, hashtable->capacity);
        return NULL;
    }

    if (hashtable->pointers) {
        return *(void **)(header + sizeof(hashtable_entry_header));
    } else {
        return header + sizeof(hashtable_entry_header);
    }
}

API void hashtable_destroy(hashtable *hashtable) {
    mem_free(hashtable->data);
    mem_zero(hashtable, sizeof(struct hashtable));
}
