/**
 * @file memory.h
 * @author Killian Bellouard (killianbellouard@gmail.com)
 * @brief This file defines the memory management system. It is responsible for allocating and freeing memory, and for tracking
 * the memory usage of the different parts of the engine, by the means of tags.
 * @version 0.1
 * @date 2024-06-11
 */

#pragma once

#include "common.h"

/** @brief The different tags for memory regions. */
typedef enum memory_tag {
    MEMORY_TAG_UNKNOWN = 0,
    MEMORY_TAG_MAX_TAGS
} memory_tag;

/**
 * @brief Initializes the memory system.
 * 
 * @note Unlike other systems, ths function will allocate the needed memory itself. As such, it must be the first system to be
 * initialized.
 * 
 * @retval TRUE Success
 * @retval FALSE Failure
 */
b8 mem_init();

/**
 * @brief Deinitializes the memory system.
 */
void mem_deinit();

/**
 * @brief Allocates a memory region of the given size and tag.
 * 
 * @param tag The tag of the memory to allocate.
 * @param size The size of the memory to allocate.
 * @return A pointer to the allocated memory region.
 */
API void* mem_alloc(memory_tag tag, u64 size);

/**
 * @brief Allocates an aligned memory region of the given size and tag.
 * 
 * @param tag The tag of the memory to allocate.
 * @param size The size of the memory to allocate.
 * @param alignment The alignment of the memory to allocate.
 * @return A pointer to the allocated memory region.
 */
API void* mem_alloc_aligned(memory_tag tag, u64 size, u64 alignment);

/**
 * @brief Frees the given memory region.
 * 
 * @param ptr The memory region to free.
 */
API void mem_free(void* ptr);

/**
 * @brief Zeroes out the given memory region.
 * 
 * @param ptr The memory region to zero.
 * @param size The size of the memory region to zero.
 */
API void mem_zero(void* ptr, u64 size);

/**
 * @brief Copies the given memory region.
 * 
 * @param dst The destination memory region.
 * @param src The source memory region.
 * @param size The size of the memory region to copy.
 */
API void mem_copy(void* dst, const void* src, u64 size);

/**
 * @brief Moves the given memory region.
 * 
 * @param dst The destination memory region.
 * @param src The source memory region.
 * @param size The size of the memory region to move.
 */
API void mem_move(void* dst, const void* src, u64 size);

/**
 * @brief Fills the given memory region with the given value.
 * 
 * @param ptr The memory region to fill.
 * @param value The value to fill the memory region with.
 * @param size The size of the memory region to fill.
 */
API void mem_set(void* ptr, u8 value, u64 size);