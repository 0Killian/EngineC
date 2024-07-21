/**
 * @file renderer_backend.h
 * @author Killian Bellouard (killianbellouard@gmail.com)
 * @brief This file defines the vulkan renderer backend interface.
 * @version 0.1
 * @date 2024-07-21
 */

#pragma once

#include <common.h>
#include <renderer/renderer_backend_interface.h>

/**
 * @brief Initializes the vulkan renderer backend.
 * 
 * @param[in] interface A pointer to the interface of the renderer backend.
 * @param[in] config A pointer to the configuration of the renderer backend.
 * @param[in] window A pointer to the window.
 * 
 * @retval TRUE Success
 * @retval FALSE Failure
 */
b8 vulkan_init(renderer_backend_interface *interface, renderer_backend_config *config, const window *window);

/**
 * @brief Deinitializes the vulkan renderer backend.
 * 
 * @param[in] interface A pointer to the interface of the renderer backend.
 */
void vulkan_deinit(renderer_backend_interface *interface);