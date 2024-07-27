#ifdef PLATFORM_LINUX
#include "vulkan_platform.h"
#include "vulkan_utils.h"
#include <core/log.h>
#include <vulkan/vulkan_wayland.h>
#include <platform/linux_adapter.h>

/**
 * @brief Creates a vulkan surface using the platform-specific implementation.
 *
 * @param[in,out] state The state of the renderer.
 * @param[in] window The window to create the surface for.
 *
 * @retval TRUE Success
 * @retval FALSE Failure
 */
b8 vulkan_platform_surface_create(vulkan_state *state, const window *window) {
    return adapter->vulkan_surface_create(state->instance, state->allocation_callbacks, (void **)&state->surface, window);
}

/**
 * @brief Appends the names of required extensions for this platform to the list of extensions.
 *
 * @param[in,out] extensions The list of extensions to append to.
 */
void vulkan_platform_get_required_extensions(extension_dynarray *extensions) {
    adapter->vulkan_get_required_extensions(extensions);
}

/**
 * @brief Indicates if the given queue on the given device supports presentation.
 *
 * @param[in] state The state of the renderer.
 * @param[in] device The device to check.
 * @param[in] queue_family_index The queue family index to check.
 *
 * @retval TRUE The queue supports presentation.
 * @retval FALSE The queue does not support presentation.
 */
b8 vulkan_platform_queue_supports_present(vulkan_state *state, VkPhysicalDevice device, u32 queue_family_index) {
    return adapter->vulkan_queue_supports_present(device, queue_family_index);
}

#endif
