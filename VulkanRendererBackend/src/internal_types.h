/**
 * @file internal_types.h
 * @author Killian Bellouard (killianbellouard@gmail.com)
 * @brief This file defines the different internal types and structures used throughout the vulkan backend.
 * @version 0.1
 * @date 2024-07-21
 */

#pragma once

#include <common.h>
#include <vulkan/vulkan.h>
#include <core/dynamic_array.h>

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
} vulkan_state;

typedef DYNARRAY(const char *) extension_dynarray;