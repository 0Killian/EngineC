#include "toml.h"
#include "dynamic_array.h"
#include "math/math.h"
#include "str.h"

#define LOG_SCOPE "TOML"
#include "core/log.h"

typedef struct toml_parser {
    toml_table *table;
    toml_table *current_parent;
    str_view content;
    u64 line;
} toml_parser;

static str_view parser_consume(toml_parser *parser, u64 size) {
    str_view result = { .begin = parser->content.begin, .size = size };

    parser->content.begin += size;
    parser->content.size -= size;

    return result;
}

static void parser_trim(toml_parser *parser, trim_type type) {
    if (type & TRIM_LEFT) {
        while (str_is_whitespace(parser->content.begin[0])) {
            if (parser->content.begin[0] == '\n') {
                parser->line++;
            }

            parser_consume(parser, 1);
        }
    }

    if (type & TRIM_RIGHT) {
        while (str_is_whitespace(parser->content.begin[parser->content.size - 1])) {
            if (parser->content.begin[parser->content.size - 1] == '\n') {
                parser->line++;
            }

            parser->content.size--;
        }
    }
}

static b8 parse_path(str_view path, toml_table *parent, toml_table_entry **result, b8 *empty);
static b8 parse_value(toml_entry *entry, toml_parser *parser);

API b8 toml_parse(const char *data, toml_table *table) {
    mem_zero(table, sizeof(toml_table));
    toml_parser parser = { .table = table, .current_parent = table, .content = str_view_from_cstr(data), .line = 1 };

    while (parser.content.size != 0) {
        parser_trim(&parser, TRIM_LEFT);

        if (str_view_starts_with_str(parser.content, "[[")) {
            parser_consume(&parser, 2);

            str_view path;
            str_view_split(&parser.content, "]]", &path);
            if (str_view_contains(path, "\n")) {
                LOG_ERROR("Invalid syntax at line %llu: invalid table name", parser.line);
                toml_free(table);
                return FALSE;
            }

            str_view_trim(&path, TRIM_BOTH);
            if (path.size == 0) {
                LOG_ERROR("Invalid syntax: \"%.*s\" -> empty table name", STR_VIEW_PRINT(parser.content));
                toml_free(table);
                return FALSE;
            }

            toml_table_entry *entry = NULL;
            b8 empty = FALSE;
            if (!parse_path(path, parser.table, &entry, &empty)) {
                toml_free(table);
                return FALSE;
            }

            if (!empty && entry->entry.type != TOML_TABLE_ENTRY_TYPE_ARRAY) {
                LOG_ERROR("Invalid syntax at line %llu: invalid entry type %d", parser.line, entry->entry.type);
                toml_free(table);
                return FALSE;
            }

            entry->entry.type = TOML_TABLE_ENTRY_TYPE_ARRAY;
            toml_entry child = { .type = TOML_TABLE_ENTRY_TYPE_TABLE, .table = {} };
            DYNARRAY_PUSH(entry->entry.array.entries, child);
            parser.current_parent = &entry->entry.array.entries.data[entry->entry.array.entries.count - 1].table;

            str_view_split(&parser.content, "\n", NULL);
            parser.line += 1;
            continue;
        }

        if (str_view_starts_with_str(parser.content, "[")) {
            parser_consume(&parser, 1);

            str_view path;
            str_view_split(&parser.content, "]", &path);
            if (str_view_contains(path, "\n")) {
                LOG_ERROR("Invalid syntax at line %llu: invalid table name", parser.line);
                toml_free(table);
                return FALSE;
            }

            str_view_trim(&path, TRIM_BOTH);
            if (path.size == 0) {
                LOG_ERROR("Invalid syntax at line: empty table name", parser.line);
                toml_free(table);
                return FALSE;
            }

            toml_table_entry *entry = NULL;
            b8 empty = FALSE;
            if (!parse_path(path, parser.table, &entry, &empty)) {
                toml_free(table);
                return FALSE;
            }

            if (!empty && entry->entry.type != TOML_TABLE_ENTRY_TYPE_TABLE) {
                LOG_ERROR("Invalid syntax at line %llu: invalid entry type %d", parser.line, entry->entry.type);
                toml_free(table);
                return FALSE;
            }

            entry->entry.type = TOML_TABLE_ENTRY_TYPE_TABLE;
            parser.current_parent = &entry->entry.table;

            str_view_split(&parser.content, "\n", NULL);
            parser.line += 1;
            continue;
        }

        if (str_view_starts_with_str(parser.content, "#")) {
            str_view_split(&parser.content, "\n", NULL);
            parser.line += 1;
            continue;
        }

        str_view key;
        str_view_split(&parser.content, "=", &key);
        if (str_view_contains(key, "\n")) {
            LOG_ERROR("Invalid syntax at line %llu: invalid key", parser.line);
            toml_free(table);
            return FALSE;
        }

        str_view_trim(&key, TRIM_BOTH);

        if (key.size == 0) {
            LOG_ERROR("Invalid syntax at line %llu: empty key", parser.line);
            toml_free(table);
            return FALSE;
        }

        toml_table_entry *entry = NULL;
        b8 empty = FALSE;
        if (!parse_path(key, parser.current_parent, &entry, &empty)) {
            toml_free(table);
            return FALSE;
        }

        if (!empty) {
            LOG_ERROR("Invalid syntax at line %llu: redefinition of key \"%.*s\"", parser.line, STR_VIEW_PRINT(key));
            toml_free(table);
            return FALSE;
        }

        parser_trim(&parser, TRIM_LEFT);
        if (!parse_value(&entry->entry, &parser)) {
            toml_free(table);
            return FALSE;
        }

        str_view_split(&parser.content, "\n", NULL);
        parser.line += 1;
    }

    return TRUE;
}

static b8 parse_path(str_view path, toml_table *parent, toml_table_entry **result, b8 *empty) {
    str_view orig_path = path;
    toml_table *current = parent;

    while (path.size != 0) {
        str_view name;
        str_view_split(&path, ".", &name);
        str_view_trim(&name, TRIM_BOTH);

        if (name.size == 0) {
            LOG_ERROR("Invalid syntax: \"%.*s\" -> empty key", STR_VIEW_PRINT(orig_path));
            return FALSE;
        }

        b8 found = FALSE;
        for (u32 i = 0; i < current->entries.count; i++) {
            if (str_view_eq(name, current->entries.data[i].key)) {
                if (path.size != 0 && current->entries.data[i].entry.type != TOML_TABLE_ENTRY_TYPE_TABLE) {
                    LOG_ERROR("Invalid syntax: \"%.*s\" -> invalid entry type %d",
                              STR_VIEW_PRINT(orig_path),
                              current->entries.data[i].entry.type);
                    return FALSE;
                } else if (path.size == 0) {
                    *result = &current->entries.data[i];
                    *empty = FALSE;
                    return TRUE;
                }

                current = &current->entries.data[i].entry.table;
                found = TRUE;
                break;
            }
        }

        if (!found) {
            if (path.size == 0) {
                toml_table_entry res = {
                    .key = str_view_dup(name),
                    .entry = {},
                };
                DYNARRAY_PUSH(current->entries, res);
                *result = &current->entries.data[current->entries.count - 1];
                *empty = TRUE;
                return TRUE;
            } else {
                toml_table_entry res = {
                    .key = str_view_dup(name),
                    .entry = {
                        .type = TOML_TABLE_ENTRY_TYPE_TABLE,
                        .table = { .entries = {} },
                    },
                };
                DYNARRAY_PUSH(current->entries, res);
                current = &current->entries.data[current->entries.count - 1].entry.table;
            }
        }
    }

    LOG_ERROR("Path is empty");
    return FALSE;
}

static b8 escape_string(char *buf, u64 *size) {
    for (u32 i = 0; i < *size; i++) {
        if (buf[i] == '\\') {
            if (i + 1 >= *size) {
                LOG_ERROR("Invalid syntax: ? -> unclosed escape sequence");
                return FALSE;
            }

            switch (buf[i + 1]) {
            case 'b': buf[i] = '\b'; break;
            case 't': buf[i] = '\t'; break;
            case 'n': buf[i] = '\n'; break;
            case 'f': buf[i] = '\f'; break;
            case 'r': buf[i] = '\r'; break;
            case '"': buf[i] = '"'; break;
            case '\\': buf[i] = '\\'; break;
            case '\n': {
                u64 to_skip = 0;
                u64 j = i + 1;
                while (j < *size) {
                    if (buf[j] == '\n' || buf[j] == ' ' || buf[j] == '\t') {
                        to_skip++;
                        j++;
                    } else {
                        break;
                    }
                }

                mem_move(buf + i, buf + j, *size - j);
                *size -= to_skip;
                goto skip;
            }
            case 'u': LOG_ERROR("Unicode not supported"); return FALSE;
            default: LOG_ERROR("Invalid syntax: \"\\%c\" -> unknown escape sequence", buf[i + 1]); return FALSE;
            }

            mem_move(buf + i + 1, buf + i + 2, *size - i - 2);
            (*size)--;
        skip:
            i++;
        }
    }
    buf[*size] = '\0';
    return TRUE;
}

static b8 parse_string(const char **result, toml_parser *parser) {
    parser_consume(parser, 1);

    str_view string;
    str_view_split(&parser->content, "\"", &string);

    char *buf = str_view_dup(string);
    u64 size = string.size;
    if (!escape_string(buf, &size)) {
        mem_free(buf);
        return FALSE;
    }

    *result = buf;
    return TRUE;
}

static b8 parse_literal(const char **result, toml_parser *parser) {
    parser_consume(parser, 1);
    str_view string;
    str_view_split(&parser->content, "'", &string);

    *result = str_view_dup(string);

    return TRUE;
}

static b8 parse_multiline_string(const char **result, toml_parser *parser) {
    parser_consume(parser, 3);
    str_view string;
    str_view_split(&parser->content, "\"\"\"", &string);

    parser->line += str_view_count(string, '\n');

    char *buf = str_view_dup(string);
    u64 size = string.size;
    if (!escape_string(buf, &size)) {
        mem_free(buf);
        return FALSE;
    }

    *result = buf;
    return TRUE;
}

static b8 parse_multiline_literal(const char **result, toml_parser *parser) {
    parser_consume(parser, 3);
    str_view string;
    str_view_split(&parser->content, "'''", &string);

    parser->line += str_view_count(string, '\n');

    *result = str_view_dup(string);
    return TRUE;
}

static b8 parse_int64(str_view string, i64 *result) {
    str_view original_string = string;
    b8 negative = str_view_starts_with(string, "-");
    if (str_view_starts_with(string, "-+")) {
        string.begin++;
        string.size--;
    }

    if (str_view_starts_with(string, "-+")) {
        LOG_ERROR("Invalid syntax: \"%.*s\" -> invalid float", STR_VIEW_PRINT(original_string));
        return FALSE;
    }

    u64 base = 10;
    if (str_view_starts_with(string, "0x")) {
        base = 16;
        string.begin += 2;
        string.size -= 2;
    } else if (str_view_starts_with(string, "0b")) {
        base = 2;
        string.begin += 2;
        string.size -= 2;
    } else if (str_view_starts_with(string, "0o")) {
        base = 8;
        string.begin += 2;
        string.size -= 2;
    }

    str_view digits_lower = {
        .begin = "0123456789abcdef",
        .size = base,
    };

    str_view digits_upper = {
        .begin = "0123456789ABCDEF",
        .size = base,
    };

    i64 value = 0;
    while (string.size != 0) {
        if (str_view_contains_char(digits_lower, string.begin[0])) {
            value = value * base + string.begin[0] - '0';
        } else if (str_view_contains_char(digits_upper, string.begin[0])) {
            value = value * base + string.begin[0] - 'A' + 10;
        } else if (string.begin[0] != '_') {
            LOG_ERROR("Invalid syntax: \"%.*s\" -> invalid integer", STR_VIEW_PRINT(original_string));
            return FALSE;
        }

        string.begin++;
        string.size--;
    }

    if (negative) {
        *result = -value;
    } else {
        *result = value;
    }

    return TRUE;
}

static b8 parse_float(str_view string, f32 *result) {
    str_view original_string = string;

    str_view integer_part;
    str_view decimal_part;
    str_view exponent_part;
    b8 negative = str_view_starts_with(string, "-");
    if (str_view_starts_with(string, "-+")) {
        string.begin++;
        string.size--;
    }

    if (str_view_starts_with(string, "-+")) {
        LOG_ERROR("Invalid syntax: \"%.*s\" -> invalid float", STR_VIEW_PRINT(original_string));
        return FALSE;
    }

    if (str_view_eq(string, "inf")) {
        *result = INFINITY;
        return TRUE;
    } else if (str_view_eq(string, "nan")) {
        *result = NAN;
        return TRUE;
    }

    if (str_view_contains(string, ".")) {
        if (!str_view_split(&string, ".", &integer_part)) {
            LOG_ERROR("Failed to split string \"%.*s\"", STR_VIEW_PRINT(original_string));
            return FALSE;
        }

        if (str_view_contains(string, "Ee")) {
            if (!str_view_split(&string, "Ee", &decimal_part)) {
                LOG_ERROR("Failed to split string \"%.*s\"", STR_VIEW_PRINT(original_string));
                return FALSE;
            }
            exponent_part = string;
        } else {
            decimal_part = string;
        }
    } else {
        if (!str_view_split(&string, "Ee", &integer_part)) {
            LOG_ERROR("Failed to split string \"%.*s\"", STR_VIEW_PRINT(original_string));
            return FALSE;
        }

        exponent_part = string;
    }

    if (integer_part.size == 0 && decimal_part.size == 0) {
        LOG_ERROR("Invalid syntax: \"%.*s\" -> invalid float", STR_VIEW_PRINT(original_string));
        return FALSE;
    }

    i64 integer = 0;
    if (!parse_int64(integer_part, &integer)) {
        LOG_ERROR("Invalid syntax: \"%.*s\" -> invalid integer", STR_VIEW_PRINT(original_string));
        return FALSE;
    }

    i64 decimal = 0;
    if (!parse_int64(decimal_part, &decimal)) {
        LOG_ERROR("Invalid syntax: \"%.*s\" -> invalid decimal", STR_VIEW_PRINT(original_string));
        return FALSE;
    }

    i64 exponent = 0;
    if (!parse_int64(exponent_part, &exponent)) {
        LOG_ERROR("Invalid syntax: \"%.*s\" -> invalid exponent", STR_VIEW_PRINT(original_string));
        return FALSE;
    }

    *result = (integer + ((f32)decimal * 0.1f)) * pow32(10.0f, (f32)exponent);

    return TRUE;
}

static b8 parse_number(toml_entry *entry, toml_parser *parser) {
    str_view string;
    str_view_take_all(&parser->content, "0123456789abcdefABCDEFx0-+ni:.", &string);

    if (parser->content.begin[-1] == '\n') {
        parser->line++;
    }

    if (str_view_contains(string, "-: ") && !str_view_starts_with(string, "-")) {
        LOG_ERROR("%.*s: Time types are not supported", STR_VIEW_PRINT(string));
        return FALSE;
    } else if ((str_view_contains(string, ".Ee") && !str_view_starts_with_str(string, "0x")) ||
               str_view_ends_with_str(string, "inf") || str_view_ends_with_str(string, "nan")) {
        entry->type = TOML_TABLE_ENTRY_TYPE_FLOAT;
        return parse_float(string, &entry->f32);
    } else {
        entry->type = TOML_TABLE_ENTRY_TYPE_INT64;
        return parse_int64(string, &entry->int64);
    }
}

static b8 parse_array(toml_array *array, toml_parser *parser) {
    parser_consume(parser, 1);
    while (TRUE) {
        parser_trim(parser, TRIM_LEFT);

        if (str_view_starts_with(parser->content, "]")) {
            parser_consume(parser, 1);
            break;
        }

        if (str_view_starts_with(parser->content, ",")) {
            parser_consume(parser, 1);
            continue;
        }

        toml_entry entry;
        DYNARRAY_PUSH(array->entries, entry);
        toml_entry *entry_ptr = &array->entries.data[array->entries.count - 1];

        parser_trim(parser, TRIM_LEFT);
        if (!parse_value(entry_ptr, parser)) {
            return FALSE;
        }
    }

    return TRUE;
}

static b8 parse_table(toml_table *table, toml_parser *parser) {
    parser_consume(parser, 1);
    *table = (toml_table){};
    while (TRUE) {
        parser_trim(parser, TRIM_LEFT);

        if (str_view_starts_with(parser->content, "}")) {
            parser_consume(parser, 1);
            break;
        }

        if (str_view_starts_with(parser->content, ",")) {
            parser_consume(parser, 1);
            continue;
        }

        str_view key;
        str_view_split(&parser->content, "=", &key);
        str_view_trim(&key, TRIM_BOTH);

        if (key.size == 0) {
            LOG_ERROR("Invalid syntax: \"%.*s\" -> empty key", STR_VIEW_PRINT(parser->content));
            return FALSE;
        }

        toml_table_entry *entry;
        b8 empty = FALSE;
        if (!parse_path(key, table, &entry, &empty)) {
            return FALSE;
        }

        if (!empty) {
            LOG_ERROR("Invalid syntax: \"%.*s\" -> duplicate key", STR_VIEW_PRINT(key));
            return FALSE;
        }

        parser_trim(parser, TRIM_LEFT);
        if (!parse_value(&entry->entry, parser)) {
            return FALSE;
        }
    }

    return TRUE;
}

static b8 parse_value(toml_entry *entry, toml_parser *parser) {
    if (str_view_starts_with_str(parser->content, "\"\"\"")) {
        entry->type = TOML_TABLE_ENTRY_TYPE_STRING;
        return parse_multiline_string(&entry->string, parser);
    } else if (str_view_starts_with_str(parser->content, "\"")) {
        entry->type = TOML_TABLE_ENTRY_TYPE_STRING;
        return parse_string(&entry->string, parser);
    } else if (str_view_starts_with_str(parser->content, "'''")) {
        entry->type = TOML_TABLE_ENTRY_TYPE_STRING;
        return parse_multiline_literal(&entry->string, parser);
    } else if (str_view_starts_with_str(parser->content, "'")) {
        entry->type = TOML_TABLE_ENTRY_TYPE_STRING;
        return parse_literal(&entry->string, parser);
    } else if (str_view_starts_with(parser->content, "0123456789-+ni")) {
        return parse_number(entry, parser);
    } else if (str_view_starts_with(parser->content, "[")) {
        entry->type = TOML_TABLE_ENTRY_TYPE_ARRAY;
        return parse_array(&entry->array, parser);
    } else if (str_view_starts_with(parser->content, "{")) {
        entry->type = TOML_TABLE_ENTRY_TYPE_TABLE;
        return parse_table(&entry->table, parser);
    } else {
        str_view value;
        str_view_split(&parser->content, " \n", &value);
        if (parser->content.begin[-1] == '\n') {
            parser->line++;
        }

        if (str_view_eq(value, "true")) {
            entry->type = TOML_TABLE_ENTRY_TYPE_BOOL;
            entry->b8 = TRUE;
            return TRUE;
        } else if (str_view_eq(value, "false")) {
            entry->type = TOML_TABLE_ENTRY_TYPE_BOOL;
            entry->b8 = FALSE;
            return TRUE;
        } else {
            LOG_ERROR("Invalid syntax at line %llu: \"%.*s\" -> invalid value (%.*s)",
                      parser->line,
                      STR_VIEW_PRINT(value),
                      STR_VIEW_PRINT(parser->content));
            return FALSE;
        }
    }
}

API toml_entry *toml_get(toml_table *table, const char *key, toml_entry_type type) {
    str_view key_view = str_view_from_cstr(key);
    toml_table *current = table;
    toml_array *current_array = NULL;

    if (key_view.size == 0) {
        return NULL;
    }

    while (key_view.size != 0) {
        toml_entry *entry;
        if (str_view_starts_with(key_view, "[")) {
            if (current_array == NULL) {
                return NULL;
            }

            key_view.begin++;
            key_view.size--;

            i64 index;
            str_view index_str;
            str_view_split(&key_view, "]", &index_str);
            parse_int64(key_view, &index);
            if (index < 0 || index >= current->entries.count) {
                return NULL;
            }

            entry = &current_array->entries.data[index];
        } else {
            if (current == NULL) {
                return NULL;
            }

            str_view name;
            str_view_take_all(&key_view, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_", &name);
            str_view_trim(&key_view, TRIM_LEFT);

            LOG_TRACE("Looking for key \"%.*s\"", STR_VIEW_PRINT(name));
            if (name.size == 0) {
                return NULL;
            }

            for (u32 i = 0; i < current->entries.count; i++) {
                if (str_view_eq(name, current->entries.data[i].key)) {
                    entry = &current->entries.data[i].entry;
                    break;
                }
            }
        }

        if (key_view.size != 0 && entry->type == TOML_TABLE_ENTRY_TYPE_TABLE) {
            current = &entry->table;
            current_array = NULL;
        } else if (key_view.size != 0 && entry->type == TOML_TABLE_ENTRY_TYPE_ARRAY) {
            current_array = &entry->array;
            current = NULL;
        } else if (key_view.size == 0 && entry->type == type) {
            return entry;
        } else {
            return NULL;
        }
        
        key_view.begin++;
        key_view.size--;
    }

    LOG_ERROR("Unreachable");
    return NULL;
}

static void toml_free_entry(toml_entry *entry) {
    if (entry->type == TOML_TABLE_ENTRY_TYPE_TABLE) {
        toml_free(&entry->table);
    } else if (entry->type == TOML_TABLE_ENTRY_TYPE_ARRAY) {
        for (u32 i = 0; i < entry->array.entries.count; i++) {
            toml_free_entry(&entry->array.entries.data[i]);
        }

        DYNARRAY_CLEAR(entry->array.entries);
    } else if (entry->type == TOML_TABLE_ENTRY_TYPE_STRING) {
        mem_free((char *)entry->string);
    }
}

API void toml_free(toml_table *table) {
    for (u32 i = 0; i < table->entries.count; i++) {
        toml_free_entry(&table->entries.data[i].entry);
        mem_free((char *)table->entries.data[i].key);
    }

    DYNARRAY_CLEAR(table->entries);
}
