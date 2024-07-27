/**
 * @file internal_types.h
 * @author Killian Bellouard (killianbellouard@gmail.com)
 * @brief This file defines the different internal types and structures used throughout the vulkan backend.
 * @version 0.1
 * @date 2024-07-21
 */

#pragma once

#include <common.h>
#include <core/dynamic_array.h>
#include <vulkan/vulkan.h>

/** @brief Structure that contains a vulkan device and other useful informations*/
typedef struct vulkan_device {
    VkPhysicalDevice physical_device;
    VkDevice logical_device;

    i32 graphics_queue_index;
    VkQueue graphics_queue;
    VkCommandPool graphics_command_pool;

    i32 present_queue_index;
    VkQueue present_queue;

    i32 transfer_queue_index;
    VkQueue transfer_queue;

    i32 compute_queue_index;
    VkQueue compute_queue;

    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceFeatures features;
    VkPhysicalDeviceMemoryProperties memory_properties;
    VkFormat depth_format;
    u8 depth_channel_count;
    b8 supports_device_local_host_visible;
    u32 surface_format_count;
    VkSurfaceFormatKHR *surface_formats;
    u32 present_mode_count;
    VkPresentModeKHR *present_modes;
} vulkan_device;

/** @brief Structure that contains a vulkan image. */
typedef struct vulkan_image {
    char *name;
    VkImage handle;
    VkImageView view;

    u32 width;
    u32 height;
    u32 depth;
    VkFormat format;

    VkDeviceMemory memory;

    VkImageLayout layout;
} vulkan_image;

/** @brief Structure that contains the swapchain informations. */
typedef struct vulkan_swapchain {
    VkSurfaceFormatKHR format;

    VkSwapchainKHR handle;

    u8 max_frames_in_flight;
    u32 image_count;
    VkImage *images;
    VkImageView *image_views;
    vulkan_image *depth_attachments;
} vulkan_swapchain;

/** @brief The state of a command buffer. */
typedef enum vulkan_command_buffer_state {
    COMMAND_BUFFER_STATE_NOT_ALLOCATED,
    COMMAND_BUFFER_STATE_READY,
    COMMAND_BUFFER_STATE_RECORDING,
    COMMAND_BUFFER_STATE_RENDERING,
    COMMAND_BUFFER_STATE_ENDED,
    COMMAND_BUFFER_STATE_SUBMITTED
} vulkan_command_buffer_state;

/** @brief Structure that contains the command buffer informations. */
typedef struct vulkan_command_buffer {
    char *name;
    VkCommandBuffer handle;
    vulkan_command_buffer_state state;
} vulkan_command_buffer;

/** @brief Structure that contains all the state of the vulkan renderer backend. */
typedef struct vulkan_state {
    VkAllocationCallbacks *allocation_callbacks;
    VkInstance instance;
    VkSurfaceKHR surface;

#ifdef DEBUG
    VkDebugUtilsMessengerEXT debug_messenger;

    PFN_vkSetDebugUtilsObjectNameEXT set_debug_utils_object_name;
    PFN_vkSetDebugUtilsObjectTagEXT set_debug_utils_object_tag;
    PFN_vkCmdBeginDebugUtilsLabelEXT cmd_begin_debug_utils_label;
    PFN_vkCmdEndDebugUtilsLabelEXT cmd_end_debug_utils_label;
#endif

    vulkan_device device;
    vulkan_swapchain swapchain;

    u32 framebuffer_width;
    u32 framebuffer_height;

    u32 current_frame;
    u32 image_index;

    vulkan_command_buffer *command_buffers;
    VkSemaphore *image_available_semaphores;
    VkSemaphore *render_finished_semaphores;
    VkFence *in_flight_fences;

    // TODO: Staging buffer
    // TODO: Samplers
    // TODO: Descriptor sets

    const struct window *win;
    uuid on_resize_handler;
} vulkan_state;

typedef DYNARRAY(const char *) extension_dynarray;
