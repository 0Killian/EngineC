#pragma once

#include "common.h"
#include "dynamic_array.h"

typedef enum toml_entry_type {
    TOML_TABLE_ENTRY_TYPE_STRING,
    TOML_TABLE_ENTRY_TYPE_INT64,
    TOML_TABLE_ENTRY_TYPE_FLOAT,
    TOML_TABLE_ENTRY_TYPE_TABLE,
    TOML_TABLE_ENTRY_TYPE_ARRAY,
    TOML_TABLE_ENTRY_TYPE_BOOL
} toml_entry_type;

typedef struct toml_table {
    DYNARRAY(struct toml_table_entry) entries;
} toml_table;

typedef struct toml_array {
    DYNARRAY(struct toml_entry) entries;
} toml_array;

typedef struct toml_entry {
    toml_entry_type type;
    union {
        const char *string;
        i64 int64;
        f32 f32;
        b8 b8;
        toml_array array;
        toml_table table;
    };
} toml_entry;

typedef struct toml_table_entry {
    const char *key;
    struct toml_entry entry;
} toml_table_entry;

API b8 toml_parse(const char *data, toml_table *table);
API toml_entry *toml_get(toml_table *table, const char *key, toml_entry_type type);
API void toml_free(toml_table *table);
