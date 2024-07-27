/**
 * @file vulkan_device.h
 * @author Killian Bellouard (killianbellouard@gmail.com)
 * @brief This file defines the vulkan device facilities.
 * @version 0.1
 * @date 2024-07-21
 */

#pragma once

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

/**
 * @brief Allocates some memory on the GPU.
 *
 * @param[in] state The state of the application.
 * @param[in] memory_requirements The memory requirements of the memory to
 * allocate.
 * @param[in] mem_props The memory properties of the memory to allocate.
 * @param[out] out_memory The allocated memory.
 *
 * @retval TRUE Success
 * @retval FALSE Failure
 */
b8 vulkan_device_mem_alloc(vulkan_state *state,
                           VkMemoryRequirements memory_requirements,
                           VkMemoryPropertyFlags mem_props,
                           VkDeviceMemory *out_memory);
