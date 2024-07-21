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

typedef struct vulkan_state {
    VkInstance instance;
} vulkan_state;