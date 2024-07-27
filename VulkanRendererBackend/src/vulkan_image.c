#include "vulkan_image.h"
#include "vulkan_device.h"
#include <core/memory.h>

#define LOG_SCOPE "VULKAN IMAGE"
#include "vulkan_utils.h"

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
                       vulkan_image *image) {

    image->width = width;
    image->height = height;
    image->depth = depth;
    image->format = format;

    VkImageCreateInfo image_create_info = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = type,
        .format = format,
        .extent = {
             .width = width,
             .height = height,
             .depth = depth,
        },
        .mipLevels = 1, // TODO: Configurable
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT, // TODO: Configurable
        .tiling = tiling,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE, // TODO: Configurable
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    };

    VkResult result =
        vkCreateImage(state->device.logical_device, &image_create_info, state->allocation_callbacks, &image->handle);
    if (result != VK_SUCCESS) {
        LOG_ERROR("Failed to create image %s: %s", name, vk_result_to_string(result));
        return FALSE;
    }

    VK_SET_OBJECT_DEBUG_NAME(state, VK_OBJECT_TYPE_IMAGE, image->handle, "Image.", name);

    VkMemoryRequirements memory_requirements;
    vkGetImageMemoryRequirements(state->device.logical_device, image->handle, &memory_requirements);
    if (!vulkan_device_mem_alloc(state, memory_requirements, mem_props, &image->memory)) {
        LOG_ERROR("Failed to allocate image memory for image %s.", name);
        return FALSE;
    }

    vkBindImageMemory(state->device.logical_device, image->handle, image->memory, 0);

    image->name = mem_alloc(MEMORY_TAG_STRING, strlen(name) + 1);
    strcpy(image->name, name);

    if (create_view) {
        if (!vulkan_image_view_create(state, image, format, view_aspect_flags)) {
            LOG_ERROR("Failed to create image view for image %s.", name);
            vulkan_image_destroy(state, image);
            return FALSE;
        }
    }

    return TRUE;
}

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
b8 vulkan_image_view_create(vulkan_state *state, vulkan_image *image, VkFormat format, VkImageAspectFlags aspect_flags) {
    VkImageViewCreateInfo image_view_create_info = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = image->handle,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = format,
        .subresourceRange = {
            .aspectMask = aspect_flags,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        },
    };

    VkResult result =
        vkCreateImageView(state->device.logical_device, &image_view_create_info, state->allocation_callbacks, &image->view);
    if (result != VK_SUCCESS) {
        LOG_ERROR("Failed to create image view: %s", vk_result_to_string(result));
        return FALSE;
    }

    VK_SET_OBJECT_DEBUG_NAME(state, VK_OBJECT_TYPE_IMAGE_VIEW, image->view, "ImageView.", image->name);
    return TRUE;
}

/**
 * @brief Destroys the image.
 *
 * @param[in] state The vulkan state.
 * @param[in] image The image.
 */
void vulkan_image_destroy(vulkan_state *state, vulkan_image *image) {
    if (image->name) {
        mem_free(image->name);
    }

    if (image->view != VK_NULL_HANDLE) {
        vkDestroyImageView(state->device.logical_device, image->view, NULL);
    }

    if (image->handle != VK_NULL_HANDLE) {
        vkDestroyImage(state->device.logical_device, image->handle, NULL);
        vkFreeMemory(state->device.logical_device, image->memory, NULL);
    }
}
