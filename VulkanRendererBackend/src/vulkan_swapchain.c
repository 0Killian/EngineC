#include "vulkan_swapchain.h"
#include "vulkan_image.h"
#include <core/memory.h>
#include <math/math.h>
#include <vulkan/vulkan.h>

#define LOG_SCOPE "VULKAN SWAPCHAIN"
#include "vulkan_utils.h"
#include <core/log.h>

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
b8 vulkan_swapchain_create(vulkan_state *state, u32 width, u32 height, vulkan_swapchain *swapchain) {
    swapchain->max_frames_in_flight = 2;

    // Chose the surface format
    swapchain->format = state->device.surface_formats[0];
    for (u32 i = 0; i < state->device.surface_format_count; i++) {
        VkSurfaceFormatKHR surface_format = state->device.surface_formats[i];
        if (surface_format.format == VK_FORMAT_B8G8R8A8_UNORM && surface_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            swapchain->format = surface_format;
            break;
        }
    }

    // Choose the present mode
    VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;
    for (u32 i = 0; i < state->device.present_mode_count; i++) {
        if (state->device.present_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
            present_mode = state->device.present_modes[i];
        }
    }

    // Choose the extent
    VkExtent2D extent = { width, height };
    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(state->device.physical_device, state->surface, &capabilities);

    if (capabilities.currentExtent.width != UINT32_MAX) {
        extent = capabilities.currentExtent;
    }

    extent.width = CLAMP(extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
    extent.height = CLAMP(extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

    // Choose the number of images
    u32 image_count = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount > 0 && image_count > capabilities.maxImageCount) {
        image_count = capabilities.maxImageCount;
    }

    // Create the swapchain
    VkSwapchainCreateInfoKHR create_info = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = state->surface,
        .minImageCount = image_count,
        .imageFormat = swapchain->format.format,
        .imageColorSpace = swapchain->format.colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .preTransform = capabilities.currentTransform,
        .presentMode = present_mode,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .clipped = VK_TRUE,
    };

    u32 queue_family_indices[] = { state->device.graphics_queue_index, state->device.present_queue_index };
    if (state->device.graphics_queue_index != state->device.present_queue_index) {
        create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        create_info.queueFamilyIndexCount = 2;
        create_info.pQueueFamilyIndices = queue_family_indices;
    } else {
        create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    if (swapchain->handle != VK_NULL_HANDLE) {
        create_info.oldSwapchain = swapchain->handle;
    }

    VkResult result =
        vkCreateSwapchainKHR(state->device.logical_device, &create_info, state->allocation_callbacks, &swapchain->handle);
    if (result != VK_SUCCESS) {
        LOG_ERROR("Failed to create swapchain: %s", vk_result_to_string(result));
        return FALSE;
    }

    VK_SET_OBJECT_DEBUG_NAME(state, VK_OBJECT_TYPE_SWAPCHAIN_KHR, swapchain->handle, "", "Swapchain");

    // Get the swapchain images
    result = vkGetSwapchainImagesKHR(state->device.logical_device, swapchain->handle, &swapchain->image_count, NULL);
    if (result != VK_SUCCESS) {
        LOG_ERROR("Failed to get swapchain images: %s", vk_result_to_string(result));
        return FALSE;
    }

    swapchain->images = mem_alloc(MEMORY_TAG_DYNARRAY, sizeof(VkImage) * swapchain->image_count);
    swapchain->depth_attachments = mem_alloc(MEMORY_TAG_DYNARRAY, sizeof(vulkan_image) * swapchain->image_count);
    swapchain->image_views = mem_alloc(MEMORY_TAG_DYNARRAY, sizeof(VkImageView) * swapchain->image_count);
    result = vkGetSwapchainImagesKHR(state->device.logical_device, swapchain->handle, &swapchain->image_count, swapchain->images);
    if (result != VK_SUCCESS) {
        LOG_ERROR("Failed to get swapchain images: %s", vk_result_to_string(result));
        mem_free(swapchain->images);
        mem_free(swapchain->image_views);
        mem_free(swapchain->depth_attachments);
        return FALSE;
    }

    // Create the image views and depth buffers
    for (u32 i = 0; i < swapchain->image_count; i++) {
        VkImageViewCreateInfo view_info = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = swapchain->images[i],
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = swapchain->format.format,
            .components = {
                .r = VK_COMPONENT_SWIZZLE_R,
                .g = VK_COMPONENT_SWIZZLE_G,
                .b = VK_COMPONENT_SWIZZLE_B,
                .a = VK_COMPONENT_SWIZZLE_A,
            },
            .subresourceRange = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
        };

        result =
            vkCreateImageView(state->device.logical_device, &view_info, state->allocation_callbacks, &swapchain->image_views[i]);
        if (result != VK_SUCCESS) {
            LOG_ERROR("Failed to create swapchain image view: %s", vk_result_to_string(result));
            mem_free(swapchain->images);
            mem_free(swapchain->image_views);
            mem_free(swapchain->depth_attachments);
            return FALSE;
        }

        char image_name[] = "SwapchainImage0";
        char view_name[] = "SwapchainImageView0";
        image_name[14] = '0' + i;
        view_name[18] = '0' + i;

        VK_SET_OBJECT_DEBUG_NAME(state, VK_OBJECT_TYPE_IMAGE, swapchain->images[i], "Image.", image_name);
        VK_SET_OBJECT_DEBUG_NAME(state, VK_OBJECT_TYPE_IMAGE_VIEW, swapchain->image_views[i], "ImageView.", view_name);

        char name[] = "SwapchainDepth0";
        name[14] = '0' + i;
        if (!vulkan_image_create(state,
                                 name,
                                 VK_IMAGE_TYPE_2D,
                                 extent.width,
                                 extent.height,
                                 1,
                                 state->device.depth_format,
                                 VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                                 VK_IMAGE_TILING_OPTIMAL,
                                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                 TRUE,
                                 VK_IMAGE_ASPECT_DEPTH_BIT,
                                 &swapchain->depth_attachments[i])) {
            mem_free(swapchain->images);
            mem_free(swapchain->image_views);
            mem_free(swapchain->depth_attachments);
            return FALSE;
        }
    }

    state->current_frame = 0;
    return TRUE;
}

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
b8 vulkan_swapchain_recreate(vulkan_state *state, u32 width, u32 height, vulkan_swapchain *swapchain) {
    VkSwapchainKHR old_handle = swapchain->handle;
    swapchain->handle = VK_NULL_HANDLE;
    vulkan_swapchain_destroy(state, swapchain);

    swapchain->handle = old_handle;
    b8 result = vulkan_swapchain_create(state, width, height, swapchain);
    vkDestroySwapchainKHR(state->device.logical_device, old_handle, state->allocation_callbacks);
    return result;
}

/**
 * @brief Destroys the swapchain.
 *
 * @param[in] state The vulkan state.
 * @param[in,out] swapchain The swapchain.
 */
void vulkan_swapchain_destroy(vulkan_state *state, vulkan_swapchain *swapchain) {
    if (swapchain->depth_attachments != NULL) {
        for (u32 i = 0; i < swapchain->image_count; i++) {
            vulkan_image_destroy(state, &swapchain->depth_attachments[i]);
        }
        mem_free(swapchain->depth_attachments);
    }

    if (swapchain->image_views != NULL) {
        for (u32 i = 0; i < swapchain->image_count; i++) {
            vkDestroyImageView(state->device.logical_device, swapchain->image_views[i], state->allocation_callbacks);
        }
        mem_free(swapchain->image_views);
    }

    if (swapchain->images != NULL) {
        mem_free(swapchain->images);
    }

    if (swapchain->handle != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(state->device.logical_device, swapchain->handle, state->allocation_callbacks);
    }
}

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
                                       u32 *image_index) {

    VkResult result = vkAcquireNextImageKHR(state->device.logical_device,
                                            swapchain->handle,
                                            UINT64_MAX,
                                            image_available_semaphore,
                                            image_acquired_fence,
                                            image_index);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        vulkan_swapchain_recreate(state, state->framebuffer_width, state->framebuffer_height, swapchain);
        return FALSE;
    } else if (result != VK_SUCCESS || result == VK_SUBOPTIMAL_KHR) {
        LOG_ERROR("Failed to acquire next swapchain image: %s", vk_result_to_string(result));
        return FALSE;
    }

    return TRUE;
}

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
                              u32 image_index) {

    VkPresentInfoKHR present_info = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &render_finished_semaphore,
        .swapchainCount = 1,
        .pSwapchains = &swapchain->handle,
        .pImageIndices = &image_index,
    };

    VkResult result = vkQueuePresentKHR(present_queue, &present_info);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        vulkan_swapchain_recreate(state, state->framebuffer_width, state->framebuffer_height, swapchain);
    } else if (result != VK_SUCCESS) {
        LOG_ERROR("Failed to present swapchain image: %s", vk_result_to_string(result));
    }
}
