#ifdef PLATFORM_WINDOWS
#include "platform/vulkan_platform.h"
#include "vulkan_utils.h"
#include <vulkan/vulkan_win32.h>
#include <windows.h>

typedef struct window_platform_state {
    HWND handle;
} window_platform_state;

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
    window_platform_state *platform_state = (window_platform_state *)window->platform_state;

    VkWin32SurfaceCreateInfoKHR create_info = {
        .sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
        .pNext = NULL,
        .flags = 0,
        .hinstance = GetModuleHandle(NULL),
        .hwnd = platform_state->handle,
    };

    VkResult result = vkCreateWin32SurfaceKHR(state->instance, &create_info, state->allocation_callbacks, &state->surface);
    if (result != VK_SUCCESS) {
        LOG_ERROR("Failed to create vulkan platform surface: %s", vk_result_to_string(result));
        return FALSE;
    }

    return TRUE;
}

/**
 * @brief Appends the names of required extensions for this platform to the list of extensions.
 *
 * @param[in,out] extensions The list of extensions to append to.
 */
void vulkan_platform_get_required_extensions(extension_dynarray *extensions) {
    DYNARRAY_PUSH(*extensions, "VK_KHR_win32_surface");
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
    return vkGetPhysicalDeviceWin32PresentationSupportKHR(device, queue_family_index) == VK_TRUE;
}

#endif
