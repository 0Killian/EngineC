/**
 * @file vulkan_utils.h
 * @author Killian Bellouard (killianbellouard@gmail.com)
 * @brief This file defines common utilities for the vulkan renderer backend.
 * @version 0.1
 * @date 2024-07-21
 */

#pragma once

#include <common.h>
#include <vulkan/vulkan.h>
#include <core/log.h>
#include "internal_types.h"

#ifdef DEBUG
#define VK_SET_OBJECT_DEBUG_NAME(state, type, object, name) do { \
        const VkDebugUtilsObjectNameInfoEXT name_info = { \
            VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT, \
            NULL, \
            type, \
            (u64)object, \
            name \
        }; \
        result = state->set_debug_utils_object_name(state->device.logical_device, &name_info); \
    } while(0)
#else
#define VK_SET_OBJECT_DEBUG_NAME()
#endif

/**
 * @brief Converts a vulkan result to a string.
 * 
 * @param[in] result The result to convert.
 * 
 * @return The string representation of the result.
 */
const char *vk_result_to_string(VkResult result);