/**
 * @file dynamic_array.h
 * @author Killian Bellouard (killianbellouard@gmail.com)
 * @brief This files defines the macros and types of the dynamic array.
 * @version 0.1
 * @date 2024-06-22
 */

#pragma once

#include "common.h"
#include "memory.h"

/**
 * @brief Defines a dynamic array.
 *
 * @param [in] type The type of the elements in the array.
 */
#define DYNARRAY(type) \
    struct {           \
        type *data;    \
        u32 count;     \
        u32 capacity;  \
    }

/**
 * @brief Clears a dynamic array, freeing its memory.
 *
 * @param [in] array The array to clear.
 */
#define DYNARRAY_CLEAR(array)       \
    do {                            \
        if ((array).data) {         \
            mem_free((array).data); \
        }                           \
        (array).data = NULL;        \
        (array).count = 0;          \
        (array).capacity = 0;       \
    } while (FALSE)

/**
 * @brief Reserves a new capacity for the array.
 *
 * @param [in] array The array to resize.
 * @param [in] capacity The new capacity of the array.
 */
#define DYNARRAY_RESERVE(array, cap)                                                          \
    do {                                                                                      \
        if ((array).data) {                                                                   \
            void *new_data = mem_alloc(MEMORY_TAG_DYNARRAY, (cap) * sizeof((array).data[0])); \
            u64 copy_size = (array).count * sizeof((array).data[0]);                          \
            mem_copy(new_data, (array).data, copy_size);                                      \
            mem_zero(new_data + copy_size, (cap) * sizeof((array).data[0]) - copy_size);      \
            mem_free((array).data);                                                           \
            (array).data = new_data;                                                          \
        } else {                                                                              \
            (array).data = mem_alloc(MEMORY_TAG_DYNARRAY, (cap) * sizeof((array).data[0]));   \
        }                                                                                     \
        (array).capacity = (cap);                                                             \
    } while (FALSE)

/**
 * @brief Pushes a new element to the end of the array.
 *
 * @param [in] array The array to push the element to.
 * @param [in] element The element to push.
 */
#define DYNARRAY_PUSH(array, element)                          \
    do {                                                       \
        if ((array).count == (array).capacity) {               \
            DYNARRAY_RESERVE(array, (array).capacity * 2 + 1); \
        }                                                      \
        (array).data[(array).count++] = (element);             \
    } while (FALSE)
