/**
 * @file hashtable.h
 * @author Killian Bellouard (killianbellouard@gmail.com)
 * @brief This files defines a hash table.
 * @version 0.1
 * @date 2024-06-27
 */

#pragma once

#include <common.h>

/** @brief A hash table, using the FNV-1a hash function. */
typedef struct hashtable {
    u32 element_size;
    u32 capacity;
    b8 pointers;
    void *data;
} hashtable;

/**
 * @brief Initializes a hashtable.
 *
 * @param[in] hashtable The hashtable to initialize.
 * @param[in] element_size The size of the elements in the hashtable.
 * @param[in] initial_capacity The initial capacity of the hashtable.
 * @param[in] pointers Whether the hashtable should store pointers or not.
 */
API void hashtable_init(hashtable *hashtable, u32 element_size, u32 initial_capacity, b8 pointers);

/**
 * @brief Inserts an element in the hashtable.
 *
 * @param[in] hashtable The hashtable to insert the element in.
 * @param[in] key The key of the element.
 * @param[in] value The value of the element.
 *
 * @retval TRUE Success
 * @retval FALSE Failure (if a collision occurs or a key already exists)
 */
API b8 hashtable_insert(hashtable *hashtable, const char *key, void *value);

/**
 * @brief Sets an element in the hashtable.
 *
 * @param[in] hashtable The hashtable to set the element in.
 * @param[in] key The key of the element.
 * @param[in] value The value of the element.
 *
 * @retval TRUE Success
 * @retval FALSE Failure
 */
API b8 hashtable_set(hashtable *hashtable, const char *key, void *value);

/**
 * @brief Gets an element from the hashtable.
 *
 * @param[in] hashtable The hashtable to get the element from.
 * @param[in] key The key of the element.
 *
 * @returns The value of the element if it exists, NULL otherwise.
 */
API void *hashtable_get(hashtable *hashtable, const char *key);

/**
 * @brief Destroys a hashtable.
 *
 * @param[in] hashtable The hashtable to destroy.
 */
API void hashtable_destroy(hashtable *hashtable);
