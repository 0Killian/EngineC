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

/**
 * @brief Converts a vulkan result to a string.
 * 
 * @param[in] result The result to convert.
 * 
 * @return The string representation of the result.
 */
const char *vk_result_to_string(VkResult result);