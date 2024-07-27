/**
 * @file vulkan_swapchain.h
 * @author Killian Bellouard (killianbellouard@gmail.com)
 * @brief This file defines the swapchain functions.
 * @version 0.1
 * @date 2024-07-21
 */

#pragma once

#include "internal_types.h"

/**
 * @brief Creates the swapchain.
 *
 * @param[in] state The vulkan state.
 * @param[in] width The width of the swapchain.
 * @param[in] height The height of the swapchain.
 * @param[out] swapchain The swapchain.
 *
 * @retval TRUE Success
 * @retval FALSE Failure
 */
b8 vulkan_swapchain_create(vulkan_state *state, u32 width, u32 height, vulkan_swapchain *swapchain);

/**
 * @brief Recreates the swapchain.
 *
 * @param[in] state The vulkan state.
 * @param[in] width The width of the swapchain.
 * @param[in] height The height of the swapchain.
 * @param[in,out] swapchain The swapchain.
 *
 * @retval TRUE Success
 * @retval FALSE Failure
 */
b8 vulkan_swapchain_recreate(vulkan_state *state, u32 width, u32 height, vulkan_swapchain *swapchain);

/**
 * @brief Destroys the swapchain.
 *
 * @param[in] state The vulkan state.
 * @param[in,out] swapchain The swapchain.
 */
void vulkan_swapchain_destroy(vulkan_state *state, vulkan_swapchain *swapchain);

/**
 * @brief Acquires an image from the swapchain.
 *
 * @param[in] state The vulkan state.
 * @param[in] swapchain The swapchain.
 * @param[in] image_available_semaphore The image available semaphore.
 * @param[in] image_acquired_fence The image acquired fence.
 * @param[out] image_index The index of the acquired image.
 *
 * @retval TRUE Success
 * @retval FALSE Failure
 */
b8 vulkan_swapchain_acquire_next_image(vulkan_state *state,
                                       vulkan_swapchain *swapchain,
                                       VkSemaphore image_available_semaphore,
                                       VkFence image_acquired_fence,
                                       u32 *image_index);

/**
 * @brief Presents an image to the swapchain.
 *
 * @param[in] state The vulkan state.
 * @param[in] swapchain The swapchain.
 * @param[in] graphics_queue The graphics queue.
 * @param[in] present_queue The present queue.
 * @param[in] render_finished_semaphore The render finished semaphore.
 * @param[in] image_index The index of the image to present.
 */
void vulkan_swapchain_present(vulkan_state *state,
                              vulkan_swapchain *swapchain,
                              VkQueue graphics_queue,
                              VkQueue present_queue,
                              VkSemaphore render_finished_semaphore,
                              u32 image_index);
