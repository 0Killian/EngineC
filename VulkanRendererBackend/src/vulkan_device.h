/**
 * @file vulkan_device.h
 * @author Killian Bellouard (killianbellouard@gmail.com)
 * @brief This file defines the vulkan device facilities.
 * @version 0.1
 * @date 2024-07-21
 */

#pragma once

#include <common.h>
#include "internal_types.h"

/**
 * @brief Select a suitable physical device, based on a score system.
 * 
 * @param[in] state The state of the renderer.
 * 
 * @retval TRUE Success
 * @retval FALSE Failure
 */
b8 vulkan_device_select(vulkan_state *state);

/**
 * @brief Create the logical device and associated objects.
 * 
 * @param[in] state The state of the renderer.
 * 
 * @retval TRUE Success
 * @retval FALSE Failure
 */
b8 vulkan_device_create(vulkan_state *state);

/**
 * @brief Destroy the logical device and associated objects.
 * 
 * @param[in] state The state of the renderer.
 */
void vulkan_device_destroy(vulkan_state *state);