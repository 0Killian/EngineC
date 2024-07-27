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

/**
 * @brief Prepares a frame for rendering.
 *
 * @param[in] interface A pointer to the interface of the renderer backend.
 * @param[in,out] packet A pointer to the frame packet.
 *
 * @retval TRUE Success
 * @retval FALSE Failure
 */
b8 vulkan_frame_prepare(renderer_backend_interface *interface, frame_packet *packet);

/**
 * @brief Begins a command list.
 *
 * @param[in] interface A pointer to the interface of the renderer backend.
 * @param[in,out] packet A pointer to the frame packet.
 *
 * @retval TRUE Success
 * @retval FALSE Failure
 */
b8 vulkan_command_list_begin(renderer_backend_interface *interface, frame_packet *packet);

/**
 * @brief Ends a command list.
 *
 * @param[in] interface A pointer to the interface of the renderer backend.
 * @param[in,out] packet A pointer to the frame packet.
 *
 * @retval TRUE Success
 * @retval FALSE Failure
 */
b8 vulkan_command_list_end(renderer_backend_interface *interface, frame_packet *packet);

/**
 * @brief Renders the current frame.
 *
 * @param[in] interface A pointer to the interface of the renderer backend.
 * @param[in,out] packet A pointer to the frame packet to render.
 */
b8 vulkan_frame_render(renderer_backend_interface *interface, frame_packet *packet);
