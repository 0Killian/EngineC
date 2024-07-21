/**
 * @file vulkan_platform.h
 * @author Killian Bellouard (killianbellouard@gmail.com)
 * @brief This file defines the platform-specific facilities for the vulkan backend.
 * @version 0.1
 * @date 2024-07-21
 */

#pragma once

#include <common.h>
#include "internal_types.h"
#include <platform/platform.h>
#include <core/dynamic_array.h>

/**
 * @brief Creates a vulkan surface using the platform-specific implementation.
 * 
 * @param[in,out] state The state of the renderer.
 * @param[in] window The window to create the surface for.
 * 
 * @retval TRUE Success
 * @retval FALSE Failure
 */
b8 vulkan_platform_surface_create(vulkan_state *state, const window *window);

/**
 * @brief Appends the names of required extensions for this platform to the list of extensions.
 * 
 * @param[in,out] extensions The list of extensions to append to.
 */
void vulkan_platform_get_required_extensions(extension_dynarray *extensions);
