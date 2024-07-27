/**
 * @file vulkan_command_buffer.h
 * @author Killian Bellouard (killianbellouard@gmail.com)
 * @brief This file defines the functions to manage command buffers.
 * @version 0.1
 * @date 2024-07-21
 */

#pragma once

#include "internal_types.h"

/**
 * @brief Allocates a command buffer from a pool.
 *
 * @param[in] state The vulkan state.
 * @param[in] pool The pool from which the command buffer should be allocated.
 * @param[in] name The name of the command buffer (may be used for debugging purposes).
 * @param[in] primary Indicates if the command buffer is a primary command buffer.
 * @param[out] command_buffer The command buffer.
 *
 * @retval TRUE Success
 * @retval FALSE Failure
 */
b8 vulkan_command_buffer_alloc(
    vulkan_state *state, VkCommandPool pool, const char *name, b8 primary, vulkan_command_buffer *command_buffer);

/**
 * @brief Frees a command buffer.
 *
 * @param[in] state The vulkan state.
 * @param[in] pool The pool from which the command buffer was allocated.
 * @param[in] command_buffer The command buffer to free.
 */
void vulkan_command_buffer_free(vulkan_state *state, VkCommandPool pool, vulkan_command_buffer *command_buffer);

/**
 * @brief Transition the command buffer into a recording state.
 *
 * @param[in] state The vulkan state.
 * @param[in] command_buffer The command buffer.
 * @param[in] is_single_use Indicates if this command buffer will be used once.
 * @param[in] is_renderpass_continue Indicates if this command buffer will be used in a renderpass.
 * @param[in] is_simultaneous_use Indicates if this command buffer will be used in multiple command buffers.
 *
 * @retval TRUE Success
 * @retval FALSE Failure
 */
b8 vulkan_command_buffer_begin(vulkan_state *state,
                               vulkan_command_buffer *command_buffer,
                               b8 is_single_use,
                               b8 is_renderpass_continue,
                               b8 is_simultaneous_use);

/**
 * @brief Ends the recording of the command buffer.
 *
 * @param[in] state The vulkan state.
 * @param[in] command_buffer The command buffer.
 *
 * @retval TRUE Success
 * @retval FALSE Failure
 */
b8 vulkan_command_buffer_end(vulkan_state *state, vulkan_command_buffer *command_buffer);

/**
 * @brief Indicates that this command buffer has been submitted.
 *
 * @note This function has no effect other than validating order of operations.
 *
 * @param[in] state The vulkan state.
 * @param[in] command_buffer The command buffer.
 */
void vulkan_command_buffer_submitted(vulkan_state *state, vulkan_command_buffer *command_buffer);

/**
 * @brief Resets a command buffer.
 *
 * @param[in] state The vulkan state.
 * @param[in] command_buffer The command buffer to reset.
 *
 * @retval TRUE Success
 * @retval FALSE Failure
 */
b8 vulkan_command_buffer_reset(vulkan_state *state, vulkan_command_buffer *command_buffer);

/**
 * @brief Allocates and begins a single-use command buffer.
 *
 * @param[in] state The vulkan state.
 * @param[in] pool The pool from which the command buffer should be allocated.
 * @param[in] name The name of the command buffer (may be used for debugging purposes).
 * @param[out] command_buffer The command buffer.
 *
 * @retval TRUE Success
 * @retval FALSE Failure
 */
b8 vulkan_command_buffer_begin_single_use(vulkan_state *state,
                                          VkCommandPool pool,
                                          const char *name,
                                          vulkan_command_buffer *command_buffer);

/**
 * @brief Ends a single-use command buffer, submits it and frees it.
 *
 * @param[in] state The vulkan state.
 * @param[in] pool The pool from which the command buffer was allocated.
 * @param[in] command_buffer The command buffer.
 * @param[in] queue The queue to submit the command buffer to.
 *
 * @retval TRUE Success
 * @retval FALSE Failure
 */
b8 vulkan_command_buffer_end_single_use(vulkan_state *state,
                                        VkCommandPool pool,
                                        vulkan_command_buffer *command_buffer,
                                        VkQueue queue);
