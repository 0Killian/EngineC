/**
 * @file vulkan_image.h
 * @author Killian Bellouard (killianbellouard@gmail.com)
 * @brief This file defines the image functions.
 * @version 0.1
 * @date 2024-07-21
 */

#pragma once

#include "internal_types.h"

// TODO: Should this function use a config struct instead ?
/**
 * @brief Creates the image.
 *
 * @param[in] state The vulkan state.
 * @param[in] name The name of the image (may be used for debugging purposes).
 * @param[in] type The type of the image (e.g. VK_IMAGE_TYPE_2D).
 * @param[in] width The width of the image.
 * @param[in] height The height of the image.
 * @param[in] depth The depth of the image.
 * @param[in] format The format of the image.
 * @param[in] usage The usage of the image.
 * @param[in] tiling The tiling of the image.
 * @param[in] mem_props The memory properties under which the image buffer should be allocated.
 * @param[in] create_view Indicates if a VkImageView should be created as well.
 * @param[in] view_aspect_flags The aspect flags of the VkImageView, if create_view is TRUE.
 * @param[out] image The image.
 *
 * @retval TRUE Success
 * @retval FALSE Failure
 */
b8 vulkan_image_create(vulkan_state *state,
                       const char *name,
                       VkImageType type,
                       u32 width,
                       u32 height,
                       u32 depth,
                       VkFormat format,
                       VkImageUsageFlags usage,
                       VkImageTiling tiling,
                       VkMemoryPropertyFlags mem_props,
                       b8 create_view,
                       VkImageAspectFlags view_aspect_flags,
                       vulkan_image *image);

/**
 * @brief Creates the image view.
 *
 * @param[in] state The vulkan state.
 * @param[in,out] image The image.
 * @param[in] format The format of the image.
 * @param[in] aspect_flags The aspect flags of the image view.
 *
 * @retval TRUE Success
 * @retval FALSE Failure
 */
b8 vulkan_image_view_create(vulkan_state *state, vulkan_image *image, VkFormat format, VkImageAspectFlags aspect_flags);

/**
 * @brief Destroys the image.
 *
 * @param[in] state The vulkan state.
 * @param[in] image The image.
 */
void vulkan_image_destroy(vulkan_state *state, vulkan_image *image);

/**
 * @brief Transitions the image layout.
 *
 * @param[in] state The vulkan state.
 * @param[in] command_buffer The command buffer.
 * @param[in] image The image.
 * @param[in] src_access_mask The source access mask.
 * @param[in] dst_access_mask The destination access mask.
 * @param[in] new_layout The new layout.
 */
void vulkan_image_transition_layout(vulkan_state *state,
                                    vulkan_command_buffer *command_buffer,
                                    vulkan_image *image,
                                    VkAccessFlags src_access_mask,
                                    VkAccessFlags dst_access_mask,
                                    VkImageLayout new_layout);
