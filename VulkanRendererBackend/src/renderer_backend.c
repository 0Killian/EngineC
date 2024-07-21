#include "renderer_backend.h"
#include "internal_types.h"
#include <core/memory.h>
#include "platform/vulkan_platform.h"

#define LOG_SCOPE "VULKAN RENDERER BACKEND"
#include <core/log.h>

static b8 create_instance(vulkan_state *state, renderer_backend_config *config);

b8 vulkan_init(renderer_backend_interface *interface, renderer_backend_config *config, const window *window) {
    vulkan_state *state = mem_alloc(MEMORY_TAG_RENDERER, sizeof(vulkan_state));
    mem_zero(state, sizeof(vulkan_state));
    interface->internal_data = state;

    if (!create_instance(state, config)) {
        return FALSE;
    }

    if (!vulkan_platform_surface_create(state, window)) {
        LOG_ERROR("Failed to create vulkan platform surface");
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

    if (state->surface != VK_NULL_HANDLE) {
        vkDestroySurfaceKHR(state->instance, state->surface, NULL);
    }

    if (state->instance != VK_NULL_HANDLE) {    
       vkDestroyInstance(state->instance, NULL);
    }

    mem_free(state);
}

static b8 create_instance(vulkan_state *state, renderer_backend_config *config) {
    VkApplicationInfo app_info = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = config->application_name,
        .applicationVersion = VK_MAKE_VERSION(0, 0, 1),
        .pEngineName = "EngineC",
        .engineVersion = VK_MAKE_VERSION(0, 0, 1),
        .apiVersion = VK_API_VERSION_1_0
    };

    #ifdef DEBUG
        uint32_t layer_count = 1;
        const char* layer_names[] = { "VK_LAYER_KHRONOS_validation" };
    #else
        uint32_t layer_count = 0;
        const char** layer_names = NULL;
    #endif

    extension_dynarray extensions = {};
    DYNARRAY_PUSH(extensions, "VK_KHR_surface");
    vulkan_platform_get_required_extensions(&extensions);

    VkInstanceCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &app_info,
        .enabledLayerCount = layer_count,
        .ppEnabledLayerNames = layer_names,
        .enabledExtensionCount = extensions.count,
        .ppEnabledExtensionNames = extensions.data
    };

    if (vkCreateInstance(&create_info, state->allocation_callbacks, &state->instance) != VK_SUCCESS) {
        LOG_ERROR("Failed to create vulkan instance");
        DYNARRAY_CLEAR(extensions);
        return FALSE;
    }

    DYNARRAY_CLEAR(extensions);
    return TRUE;
}