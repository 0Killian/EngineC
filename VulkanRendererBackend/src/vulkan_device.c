#include "internal_types.h"
#include "platform/vulkan_platform.h"
#include <string.h>

#define LOG_SCOPE "VULKAN DEVICE"
#include <core/log.h>

#include "vulkan_utils.h"

static b8 select_queue_indices(vulkan_state *state, vulkan_device *device);
static b8 select_depth_format(vulkan_state *state, vulkan_device *device);

/**
 * @brief Select a suitable physical device, based on a score system.
 * 
 * @param[in] state The state of the renderer.
 * 
 * @retval TRUE Success
 * @retval FALSE Failure
 */
b8 vulkan_device_select(vulkan_state *state) {
    VkResult result;
    
    u32 physical_device_count = 0;
    result = vkEnumeratePhysicalDevices(state->instance, &physical_device_count, NULL);
    if (result != VK_SUCCESS) {
        LOG_ERROR("Failed to enumerate physical devices: %s", vk_result_to_string(result));
        return FALSE;
    }

    if (physical_device_count == 0) {
        LOG_ERROR("Failed to find GPUs with Vulkan support!");
        return FALSE;
    }

    VkPhysicalDevice *physical_devices = mem_alloc(MEMORY_TAG_DYNARRAY, sizeof(VkPhysicalDevice) * physical_device_count);
    result = vkEnumeratePhysicalDevices(state->instance, &physical_device_count, physical_devices);
    if (result != VK_SUCCESS) {
        LOG_ERROR("Failed to enumerate physical devices: %s", vk_result_to_string(result));
        mem_free(physical_devices);
        return FALSE;
    }

    i64 best_score = -1;
    vulkan_device best_device = {};

    for (u32 i = 0; i < physical_device_count; i++) {
        vulkan_device device = {};
        device.physical_device = physical_devices[i];
        vkGetPhysicalDeviceProperties(device.physical_device, &device.properties);
        vkGetPhysicalDeviceFeatures(device.physical_device, &device.features);
        vkGetPhysicalDeviceMemoryProperties(device.physical_device, &device.memory_properties);
        if (!select_queue_indices(state, &device)) {
            mem_free(physical_devices);
            return FALSE;
        }

        if (!select_depth_format(state, &device)) {
            mem_free(physical_devices);
            return FALSE;
        }

        for (u32 i = 0; i < device.memory_properties.memoryTypeCount; i++) {
            if (device.memory_properties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT &&
                device.memory_properties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
                device.supports_device_local_host_visible = TRUE;
                break;
            }
        }

        if (device.graphics_queue_index == -1 || device.present_queue_index == -1 || device.transfer_queue_index == -1) {
            continue;
        }

        i64 score = 0;

        if (device.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            score += 10000;
        }

        score += device.properties.limits.maxImageDimension2D;

        if (device.supports_device_local_host_visible) {
            score += 100;
        }

        if (score > best_score) {
            best_score = score;
            best_device = device;
        }
    }

    if (best_device.physical_device == VK_NULL_HANDLE) {
        LOG_ERROR("Failed to find a suitable GPU!");
        mem_free(physical_devices);
        return FALSE;
    }

    state->device = best_device;

    LOG_INFO("Selected device: %s", state->device.properties.deviceName);

    mem_free(physical_devices);
    return TRUE;
}

/**
 * @brief Create the logical device and associated objects.
 * 
 * @param[in] state The state of the renderer.
 * 
 * @retval TRUE Success
 * @retval FALSE Failure
 */
b8 vulkan_device_create(vulkan_state *state) {
    VkResult result;

    VkDeviceQueueCreateInfo queue_create_infos[3] = {};
    u32 queue_count = 0;
    f32 queue_priorities[2] = { 0.9f, 1.0f };

    u32 present_queue_index;
    u32 transfer_queue_index;

    // Graphics queue
    u32 graphics_queue_index = 0;
    queue_create_infos[queue_count].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_infos[queue_count].queueFamilyIndex = state->device.graphics_queue_index;
    queue_create_infos[queue_count].queueCount = 1;
    queue_create_infos[queue_count].pNext = NULL;
    queue_create_infos[queue_count].pQueuePriorities = queue_priorities;
    if (state->device.graphics_queue_index == state->device.present_queue_index) {
        present_queue_index = queue_create_infos[queue_count].queueCount;
        queue_create_infos[queue_count].queueCount += 1;
    }

    if (state->device.graphics_queue_index == state->device.transfer_queue_index) {
        transfer_queue_index = queue_create_infos[queue_count].queueCount;
        queue_create_infos[queue_count].queueCount += 1;
    }
    queue_count++;

    // Present queue
    if (state->device.graphics_queue_index != state->device.present_queue_index) {
        present_queue_index = 0;
        queue_create_infos[queue_count].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_create_infos[queue_count].queueFamilyIndex = state->device.present_queue_index;
        queue_create_infos[queue_count].queueCount = 1;
        queue_create_infos[queue_count].pNext = NULL;
        queue_create_infos[queue_count].pQueuePriorities = queue_priorities + 1;
        queue_count++;
    }

    // Transfer queue
    if (state->device.graphics_queue_index != state->device.transfer_queue_index) {
        transfer_queue_index = 0;
        queue_create_infos[queue_count].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_create_infos[queue_count].queueFamilyIndex = state->device.transfer_queue_index;
        queue_create_infos[queue_count].queueCount = 1;
        queue_create_infos[queue_count].pNext = NULL;
        queue_create_infos[queue_count].pQueuePriorities = queue_priorities + 1;
        queue_count++;
    }

    u32 available_extensions_count = 0;
    result = vkEnumerateDeviceExtensionProperties(state->device.physical_device, NULL, &available_extensions_count, NULL);
    VkExtensionProperties *available_extensions = mem_alloc(MEMORY_TAG_DYNARRAY, sizeof(VkExtensionProperties) * available_extensions_count);
    result = vkEnumerateDeviceExtensionProperties(state->device.physical_device, NULL, &available_extensions_count, available_extensions);

    u32 required_extensions_count = 1;
    const char *required_extensions[1] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

    for (u32 i = 0; i < required_extensions_count; i++) {
        b8 found = FALSE;
        for (u32 j = 0; j < available_extensions_count; j++) {
            // TODO: String functions
            if (strcmp(required_extensions[i], available_extensions[j].extensionName) == 0) {
                found = TRUE;
                break;
            }
        }

        if (!found) {
            LOG_ERROR("Missing required device extension: %s", required_extensions[i]);
            mem_free(available_extensions);
            return FALSE;
        }
    }

    mem_free(available_extensions);

    VkDeviceCreateInfo device_create_info = {};
    device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_create_info.enabledExtensionCount = required_extensions_count;
    device_create_info.ppEnabledExtensionNames = required_extensions;
    device_create_info.queueCreateInfoCount = queue_count;
    device_create_info.pQueueCreateInfos = queue_create_infos;
    
    result = vkCreateDevice(
        state->device.physical_device,
        &device_create_info,
        state->allocation_callbacks,
        &state->device.logical_device);
    if (result != VK_SUCCESS) {
        LOG_ERROR("Failed to create logical device!");
        return FALSE;
    }

    VK_SET_OBJECT_DEBUG_NAME(state, VK_OBJECT_TYPE_DEVICE, state->device.logical_device, state->device.properties.deviceName);

    // Get queues
    vkGetDeviceQueue(
        state->device.logical_device,
        state->device.graphics_queue_index,
        graphics_queue_index,
        &state->device.graphics_queue);
    
    vkGetDeviceQueue(
        state->device.logical_device,
        state->device.present_queue_index,
        present_queue_index,
        &state->device.present_queue);

    vkGetDeviceQueue(
        state->device.logical_device,
        state->device.transfer_queue_index,
        transfer_queue_index,
        &state->device.transfer_queue);

    // Create command pool
    VkCommandPoolCreateInfo command_pool_create_info = {};
    command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    command_pool_create_info.queueFamilyIndex = state->device.graphics_queue_index;
    command_pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    result = vkCreateCommandPool(
        state->device.logical_device,
        &command_pool_create_info,
        state->allocation_callbacks,
        &state->device.graphics_command_pool);
    if (result != VK_SUCCESS) {
        LOG_ERROR("Failed to create command pool!");
        return FALSE;
    }

    return TRUE;
}

/**
 * @brief Destroy the logical device and associated objects.
 * 
 * @param[in] state The state of the renderer.
 */
void vulkan_device_destroy(vulkan_state *state) {
    if (state->device.graphics_command_pool != VK_NULL_HANDLE) {
        vkDestroyCommandPool(state->device.logical_device, state->device.graphics_command_pool, state->allocation_callbacks);
    }

    if (state->device.logical_device != VK_NULL_HANDLE) {
        vkDestroyDevice(state->device.logical_device, state->allocation_callbacks);
    }
}

static b8 select_queue_indices(vulkan_state *state, vulkan_device *device) {
    VkResult result;
    // Indicates the probability that the transfer queue is dedicated to transfers (lower is better)
    u8 min_transfer_score = 255;

    u32 queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device->physical_device, &queue_family_count, NULL);

    VkQueueFamilyProperties *queue_families = mem_alloc(MEMORY_TAG_DYNARRAY, sizeof(VkQueueFamilyProperties) * queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(device->physical_device, &queue_family_count, queue_families);

    for (u32 i = 0; i < queue_family_count; i++) {
        u8 score = 0;
        b8 supports_present = vulkan_platform_queue_supports_present(state, device->physical_device, i);

        if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            score++;

            // Prefer graphics and present-combined queues
            if (supports_present) {
                device->graphics_queue_index = i;
            } else if (device->graphics_queue_index == -1) {
                device->graphics_queue_index = i;
            }
        }

        if (supports_present) {
            score++;
            if (device->present_queue_index == -1) {
                device->present_queue_index = i;
            }
        }

        if (queue_families[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
            score++;
            if (device->transfer_queue_index == -1) {
                device->transfer_queue_index = i;
            }
        }

        if (queue_families[i].queueFlags & VK_QUEUE_TRANSFER_BIT) {
            if (score < min_transfer_score) {
                min_transfer_score = score;
                device->transfer_queue_index = i;
            }
        }
    }

    mem_free(queue_families);
    return TRUE;
}

static b8 select_depth_format(vulkan_state *state, vulkan_device *device) {
    VkFormat candidates[2] = {VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT};
    u32 channel_counts[2] = {4, 3};

    u32 flags = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
    for (u64 i = 0; i < 2; i++) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(device->physical_device, candidates[i], &props);
        
        if ((props.linearTilingFeatures & flags) == flags || (props.optimalTilingFeatures & flags) == flags) {
            device->depth_format = candidates[i];
            device->depth_channel_count = 4;
            return TRUE;
        }
    }

    LOG_ERROR("Failed to find supported depth format");
    return FALSE;
}