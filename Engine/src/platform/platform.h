/**
 * @file platform.h
 * @author Killian Bellouard (killianbellouard@gmail.com)
 * @brief This file defines the interface of the platform layer. Each platform must implement this interface in a separate .c
 * file.
 * @version 0.1
 * @date 2024-06-11
 */

#pragma once

#include "common.h"

/**
 * @brief Initializes the platform layer.
 * 
 * Should be called twice, once to get the required allocation size (with state == NULL), and a second time to actually
 * initialize the platform (with state != NULL).
 * 
 * @param[in] state A pointer to a memory region to store the state of the platform layer. To obtain the needed size, pass NULL.
 * @param[out] size_requirement A pointer to the size of the memory that should be allocated.
 * @retval TRUE Success
 * @retval FALSE Failure
 */
API b8 platform_init(void* state, u64* size_requirement);

/**
 * @brief Deinitializes the platform layer.
 * 
 * @param[in] state A pointer to the state of the platform layer.
 */
API void platform_deinit(void* state);
