#include "str.h"
#include "memory.h"
#include <string.h>

str_view str_view_from_cstr(const char *cstr) { return (str_view){ .begin = cstr, .size = strlen(cstr) }; }

b8 str_view_split(str_view *str, const char *delims, str_view *out) {
    if (out != NULL) {
        out->begin = str->begin;
        out->size = 0;
    }
    u32 delim_count = strlen(delims);

    b8 found = FALSE;
    while (str->size > 0) {
        for (u32 i = 0; i < delim_count; i++) {
            if (str->begin[0] == delims[i]) {
                found = TRUE;
                break;
            }
        }

        str->begin++;
        str->size--;

        if (found) {
            break;
        }

        if (out != NULL) {
            out->size++;
        }
    }

    return found;
}

b8 str_view_take_all(str_view *view, const char *characters, str_view *out) {
    *out = (str_view) {
        .begin = view->begin,
        .size = 0,
    };

    while (view->size > 0) {
        b8 found = FALSE;
        for (u32 i = 0; characters[i] != 0; i++) {
            if (view->begin[0] == characters[i]) {
                found = TRUE;
                break;
            }
        }

        if (found) {
            view->begin++;
            view->size--;
            (*out).size++;
        } else {
            return FALSE;
        }
    }

    return TRUE;
}

void str_view_trim(str_view *view, trim_type type) {
    if (type & TRIM_LEFT) {
        while (view->size > 0 && str_is_whitespace(view->begin[0])) {
            view->begin++;
            view->size--;
        }
    }

    if (type & TRIM_RIGHT) {
        while (view->size > 0 && view->begin[view->size - 1] == ' ') {
            view->size--;
        }
    }
}

b8 str_view_eq_view(str_view a, str_view b) { return a.size == b.size && memcmp(a.begin, b.begin, a.size) == 0; }

b8 str_view_eqi_view(str_view a, str_view b) { return a.size == b.size && strcasecmp(a.begin, b.begin) == 0; }

b8 str_view_eq(str_view a, const char *b) { return a.size == strlen(b) && memcmp(a.begin, b, a.size) == 0; }

b8 str_view_eqi(str_view a, const char *b) { return a.size == strlen(b) && strcasecmp(a.begin, b) == 0; }

b8 str_eq(const char *a, const char *b) { return strcmp(a, b) == 0; }

b8 str_eqi(const char *a, const char *b) { return strcasecmp(a, b) == 0; }

char *str_view_dup(str_view view) {
    char *dup = mem_alloc(MEMORY_TAG_STRING, view.size + 1);
    mem_copy(dup, view.begin, view.size);
    dup[view.size] = 0;
    return dup;
}

char *str_dup(const char *str) {
    u64 size = strlen(str) + 1;
    char *dup = mem_alloc(MEMORY_TAG_STRING, size);
    mem_copy(dup, str, size);
    return dup;
}

b8 str_view_starts_with_str(str_view view, const char *prefix) {
    u64 prefix_size = strlen(prefix);
    return view.size >= prefix_size && memcmp(view.begin, prefix, prefix_size) == 0;
}

b8 str_view_ends_with_str(str_view view, const char *suffix) {
    u64 suffix_size = strlen(suffix);
    return view.size >= suffix_size && memcmp(view.begin + view.size - suffix_size, suffix, suffix_size) == 0;
}

b8 str_view_starts_with(str_view view, const char *delims) {
    u64 delim_count = strlen(delims);
    for (u32 i = 0; i < delim_count; i++) {
        if (view.begin[0] == delims[i]) {
            return TRUE;
        }
    }
    return FALSE;
}

void str_cat_view_alloc(char **dest, str_view view) {
    char *new = mem_alloc(MEMORY_TAG_STRING, (*dest ? strlen(*dest) : 0) + view.size + 1);
    if (*dest) {
        mem_copy(new, *dest, strlen(*dest));
    }

    str_cat_view(new, view);

    if (*dest) {
        mem_free(*dest);
    }
    *dest = new;
}

void str_cat_view(char *dest, str_view view) {
    u64 dest_size = strlen(dest);
    u64 len = view.size;
    mem_copy(dest + dest_size, view.begin, len);
    dest[dest_size + len] = 0;
}

void str_cat_alloc(char **dest, const char *str) {
    char *new = mem_alloc(MEMORY_TAG_STRING, strlen(*dest) + strlen(str) + 1);
    if (*dest) {
        mem_copy(new, *dest, strlen(*dest));
    }

    str_cat(new, str);

    if (*dest) {
        mem_free(*dest);
    }
    *dest = new;
}

void str_cat(char *dest, const char *str) {
    u64 dest_size = strlen(dest);
    u64 len = strlen(str);
    mem_copy(dest + dest_size, str, len);
    dest[dest_size + len] = 0;
}

b8 str_contains_str(const char *haystack, const char *needle) { return strstr(haystack, needle) != NULL; }

b8 str_view_contains(str_view haystack, const char *needles) {
    for (u32 i = 0; i < haystack.size; i++) {
        for (u32 j = 0; needles[j] != 0; j++) {
            if (haystack.begin[i] == needles[j]) {
                return TRUE;
            }
        }
    }

    return FALSE;
}

b8 str_view_contains_char(str_view haystack, char needle) {
    for (u32 i = 0; i < haystack.size; i++) {
        if (haystack.begin[i] == needle) {
            return TRUE;
        }
    }

    return FALSE;
}

u64 str_len(const char *str) { return strlen(str); }

u64 str_view_count(str_view view, char needle) {
    u64 count = 0;
    for (u32 i = 0; i < view.size; i++) {
        if (view.begin[i] == needle) {
            count++;
        }
    }
    return count;
}
