#include "renderer_backend.h"
#include "internal_types.h"
#include <core/memory.h>

#define LOG_SCOPE "VULKAN RENDERER BACKEND"
#include <core/log.h>

b8 vulkan_init(renderer_backend_interface *interface, renderer_backend_config *config) {
    vulkan_state *state = mem_alloc(MEMORY_TAG_RENDERER, sizeof(vulkan_state));
    mem_zero(state, sizeof(vulkan_state));
    interface->internal_data = state;

    VkApplicationInfo app_info = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "Vulkan Renderer Backend",
        .applicationVersion = VK_MAKE_VERSION(0, 0, 1),
        .pEngineName = "Vulkan Renderer Backend",
        .engineVersion = VK_MAKE_VERSION(0, 0, 1),
        .apiVersion = VK_API_VERSION_1_0
    };

    VkInstanceCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &app_info
    };

    if (vkCreateInstance(&create_info, NULL, &state->instance) != VK_SUCCESS) {
        LOG_ERROR("Failed to create vulkan instance");
        return FALSE;
    }

    LOG_INFO("Vulkan renderer backend initialized");

    return TRUE;
}

void vulkan_deinit(renderer_backend_interface *interface) {
    vulkan_state *state = (vulkan_state*)interface->internal_data;

    if (state == NULL) {
        LOG_ERROR("Vulkan renderer backend not initialized");
        return;
    }

    if (state->instance != VK_NULL_HANDLE) {    
       vkDestroyInstance(state->instance, NULL);
    }

    mem_free(state);
}