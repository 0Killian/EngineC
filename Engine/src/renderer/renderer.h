/**
 * @file plugins.h
 * @author Killian Bellouard (killianbellouard@gmail.com)
 * @brief This file defines the interface to the renderer system, specifically the "frontend" part
 * of the renderer.
 * @version 0.1
 * @date 2024-07-21
 */

#pragma once

#include "common.h"
#include "platform/platform.h"

/**
 * @brief Initializes the renderer system.
 * 
 * Should be called twice, once to get the required allocation size (with state == NULL), and a second time to actually
 * initialize the renderer system (with state != NULL).
 * 
 * @note The renderer system is dependent on the event and plugins systems, and should be initialized after them.
 * 
 * @param[in] state A pointer to a memory region to store the state of the renderer system. To obtain the needed size, pass NULL.
 * @param[out] size_requirement A pointer to the size of the memory that should be allocated.
 * @param[in] window A pointer to the window to use for the renderer.
 * 
 * @retval TRUE Success
 * @retval FALSE Failure
 */
b8 renderer_init(void *state, u64 *size_requirement, const window *window);

/**
 * @brief Deinitializes the renderer system.
 * 
 * @param[in] state A pointer to the state of the renderer system.
 */
void renderer_deinit(void *state);