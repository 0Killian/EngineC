#include "internal_types.h"
#include "platform/vulkan_platform.h"
#include <string.h>
#include <vulkan/vulkan.h>

#define LOG_SCOPE "VULKAN DEVICE"
#include <core/log.h>

#include "vulkan_utils.h"

static b8 select_queue_indices(vulkan_state *state, vulkan_device *device);
static b8 select_depth_format(vulkan_state *state, vulkan_device *device);

static const char *needed_extensions[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
static const u32 needed_extension_count = sizeof(needed_extensions) / sizeof(needed_extensions[0]);

VkPhysicalDeviceDynamicRenderingFeatures dynamic_rendering_features = {
    .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES,
    .pNext = NULL,
    .dynamicRendering = VK_TRUE,
};

VkDeviceCreateInfo device_create_info = {
    .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
    .pNext = &dynamic_rendering_features,
};

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

        u32 available_extension_count = 0;
        result = vkEnumerateDeviceExtensionProperties(device.physical_device, NULL, &available_extension_count, NULL);
        if (result != VK_SUCCESS) {
            LOG_ERROR("Failed to enumerate device extensions: %s", vk_result_to_string(result));
            mem_free(physical_devices);
            return FALSE;
        }

        if (available_extension_count == 0) {
            LOG_ERROR("Failed to find device extensions");
            mem_free(physical_devices);
            return FALSE;
        }

        VkExtensionProperties *available_extensions = mem_alloc(MEMORY_TAG_DYNARRAY, sizeof(VkExtensionProperties) * available_extension_count);
        result = vkEnumerateDeviceExtensionProperties(
            device.physical_device, NULL, &available_extension_count, available_extensions);
        if (result != VK_SUCCESS) {
            LOG_ERROR("Failed to enumerate device extensions: %s", vk_result_to_string(result));
            mem_free(available_extensions);
            mem_free(physical_devices);
            return FALSE;
        }

        b8 found_extensions = TRUE;
        for (u32 j = 0; j < needed_extension_count; j++) {
            b8 found = FALSE;
            for (u32 k = 0; k < available_extension_count; k++) {
                if (strcmp(needed_extensions[j], available_extensions[k].extensionName) == 0) {
                    found = TRUE;
                    break;
                }
            }

            if (!found) {
                mem_free(available_extensions);
                found_extensions = FALSE;
                break;
            }
        }

        if (!found_extensions) {
            continue;
        }

        mem_free(available_extensions);

        result = vkGetPhysicalDeviceSurfaceFormatsKHR(device.physical_device, state->surface, &device.surface_format_count, NULL);
        if (result != VK_SUCCESS) {
            LOG_ERROR("Failed to get physical device surface formats: %s", vk_result_to_string(result));
            mem_free(physical_devices);
            return FALSE;
        }

        device.surface_formats = mem_alloc(MEMORY_TAG_RENDERER, sizeof(VkSurfaceFormatKHR) * device.surface_format_count);
        result = vkGetPhysicalDeviceSurfaceFormatsKHR(
            device.physical_device, state->surface, &device.surface_format_count, device.surface_formats);
        if (result != VK_SUCCESS) {
            LOG_ERROR("Failed to get physical device surface formats: %s", vk_result_to_string(result));
            mem_free(physical_devices);
            return FALSE;
        }

        result =
            vkGetPhysicalDeviceSurfacePresentModesKHR(device.physical_device, state->surface, &device.present_mode_count, NULL);
        if (result != VK_SUCCESS) {
            LOG_ERROR("Failed to get physical device surface present modes: %s", vk_result_to_string(result));
            mem_free(device.surface_formats);
            mem_free(physical_devices);
            return FALSE;
        }

        device.present_modes = mem_alloc(MEMORY_TAG_RENDERER, sizeof(VkPresentModeKHR) * device.present_mode_count);
        result = vkGetPhysicalDeviceSurfacePresentModesKHR(
            device.physical_device, state->surface, &device.present_mode_count, device.present_modes);
        if (result != VK_SUCCESS) {
            LOG_ERROR("Failed to get physical device surface present modes: %s", vk_result_to_string(result));
            mem_free(device.present_modes);
            mem_free(device.surface_formats);
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
            mem_free(device.present_modes);
            mem_free(device.surface_formats);
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
        } else {
            mem_free(device.present_modes);
            mem_free(device.surface_formats);
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

    device_create_info.enabledExtensionCount = needed_extension_count;
    device_create_info.ppEnabledExtensionNames = needed_extensions;
    device_create_info.queueCreateInfoCount = queue_count;
    device_create_info.pQueueCreateInfos = queue_create_infos;

    result = vkCreateDevice(
        state->device.physical_device, &device_create_info, state->allocation_callbacks, &state->device.logical_device);
    if (result != VK_SUCCESS) {
        LOG_ERROR("Failed to create logical device!");
        return FALSE;
    }

    VK_SET_OBJECT_DEBUG_NAME(
        state, VK_OBJECT_TYPE_DEVICE, state->device.logical_device, "Device.", state->device.properties.deviceName);

    // Get queues
    vkGetDeviceQueue(
        state->device.logical_device, state->device.graphics_queue_index, graphics_queue_index, &state->device.graphics_queue);

    vkGetDeviceQueue(
        state->device.logical_device, state->device.present_queue_index, present_queue_index, &state->device.present_queue);

    vkGetDeviceQueue(
        state->device.logical_device, state->device.transfer_queue_index, transfer_queue_index, &state->device.transfer_queue);

    // Create command pool
    VkCommandPoolCreateInfo command_pool_create_info = {};
    command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    command_pool_create_info.queueFamilyIndex = state->device.graphics_queue_index;
    command_pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    result = vkCreateCommandPool(state->device.logical_device,
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

    if (state->device.present_modes != NULL) {
        mem_free(state->device.present_modes);
    }

    if (state->device.surface_formats != NULL) {
        mem_free(state->device.surface_formats);
    }

    mem_zero(&state->device, sizeof(vulkan_device));
}

/**
 * @brief Allocates some memory on the GPU.
 *
 * @param[in] state The state of the application.
 * @param[in] memory_requirements The memory requirements of the memory to
 * allocate.
 * @param[in] mem_props The memory properties of the memory to allocate.
 * @param[out] out_memory The allocated memory.
 *
 * @retval TRUE Success
 * @retval FALSE Failure
 */
b8 vulkan_device_mem_alloc(vulkan_state *state,
                           VkMemoryRequirements memory_requirements,
                           VkMemoryPropertyFlags mem_props,
                           VkDeviceMemory *out_memory) {
    i32 memory_type_index = -1;
    for (u32 i = 0; i < state->device.memory_properties.memoryTypeCount; i++) {
        if ((memory_requirements.memoryTypeBits & (1 << i)) &&
            (state->device.memory_properties.memoryTypes[i].propertyFlags & mem_props) == mem_props) {
            memory_type_index = i;
            break;
        }
    }

    if (memory_type_index == -1) {
        LOG_ERROR("Failed to find suitable memory type!");
        return FALSE;
    }

    VkMemoryAllocateInfo memory_allocate_info = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = memory_requirements.size,
        .memoryTypeIndex = memory_type_index,
    };

    VkResult result =
        vkAllocateMemory(state->device.logical_device, &memory_allocate_info, state->allocation_callbacks, out_memory);
    if (result != VK_SUCCESS) {
        LOG_ERROR("Failed to allocate memory!");
        return FALSE;
    }

    return TRUE;
}

static b8 select_queue_indices(vulkan_state *state, vulkan_device *device) {
    VkResult result;
    // Indicates the probability that the transfer queue is dedicated to transfers
    // (lower is better)
    u8 min_transfer_score = 255;

    u32 queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device->physical_device, &queue_family_count, NULL);

    VkQueueFamilyProperties *queue_families =
        mem_alloc(MEMORY_TAG_DYNARRAY, sizeof(VkQueueFamilyProperties) * queue_family_count);
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
    VkFormat candidates[2] = { VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT };
    u32 channel_counts[2] = { 4, 3 };

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
