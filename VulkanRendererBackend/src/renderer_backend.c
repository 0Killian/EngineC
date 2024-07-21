#include "renderer_backend.h"
#include "internal_types.h"
#include <core/memory.h>
#include "platform/vulkan_platform.h"

#define LOG_SCOPE "VULKAN RENDERER BACKEND"
#include <core/log.h>

#include "vulkan_utils.h"

#define GET_PROC_ADDR(var, name) \
    var = (PFN_##name) vkGetInstanceProcAddr(state->instance, #name); \
    if (var == NULL) { \
        LOG_ERROR("Failed to get " #name " function address"); \
        return FALSE; \
    }

static b8 create_instance(vulkan_state *state, renderer_backend_config *config);
static b8 debug_setup(vulkan_state *state);

static VkBool32 VKAPI_PTR debug_messenger_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageTypes,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
    void *pUserData);

b8 vulkan_init(renderer_backend_interface *interface, renderer_backend_config *config, const window *window) {
    vulkan_state *state = mem_alloc(MEMORY_TAG_RENDERER, sizeof(vulkan_state));
    mem_zero(state, sizeof(vulkan_state));
    interface->internal_data = state;

    if (!create_instance(state, config)) {
        return FALSE;
    }

    #ifdef DEBUG
    if (!debug_setup(state)) {
        return FALSE;
    }
    #endif

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
        vkDestroySurfaceKHR(state->instance, state->surface, state->allocation_callbacks);
    }

    #ifdef DEBUG
        if (state->debug_messenger != VK_NULL_HANDLE) {
            PFN_vkDestroyDebugUtilsMessengerEXT destroy_debug_utils_messenger =
                (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(state->instance, "vkDestroyDebugUtilsMessengerEXT");
            destroy_debug_utils_messenger(state->instance, state->debug_messenger, state->allocation_callbacks);
        }
    #endif

    if (state->instance != VK_NULL_HANDLE) {    
       vkDestroyInstance(state->instance, state->allocation_callbacks);
    }

    mem_free(state);
}

static b8 create_instance(vulkan_state *state, renderer_backend_config *config) {
    VkResult result;

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
    DYNARRAY_PUSH(extensions, VK_KHR_SURFACE_EXTENSION_NAME);
    #ifdef DEBUG
    DYNARRAY_PUSH(extensions, VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    #endif
    vulkan_platform_get_required_extensions(&extensions);

    VkInstanceCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &app_info,
        .enabledLayerCount = layer_count,
        .ppEnabledLayerNames = layer_names,
        .enabledExtensionCount = extensions.count,
        .ppEnabledExtensionNames = extensions.data
    };

    result = vkCreateInstance(&create_info, state->allocation_callbacks, &state->instance);
    if (result != VK_SUCCESS) {
        LOG_ERROR("Failed to create vulkan instance: %s", vk_result_to_string(result));
        DYNARRAY_CLEAR(extensions);
        return FALSE;
    }

    DYNARRAY_CLEAR(extensions);
    return TRUE;
}

static b8 debug_setup(vulkan_state *state) {
    VkResult result;

    PFN_vkCreateDebugUtilsMessengerEXT create_debug_utils_messenger = NULL;
    GET_PROC_ADDR(create_debug_utils_messenger, vkCreateDebugUtilsMessengerEXT);
    GET_PROC_ADDR(state->set_debug_utils_object_name, vkSetDebugUtilsObjectNameEXT);
    GET_PROC_ADDR(state->set_debug_utils_object_tag, vkSetDebugUtilsObjectTagEXT);
    GET_PROC_ADDR(state->cmd_begin_debug_utils_label, vkCmdBeginDebugUtilsLabelEXT);
    GET_PROC_ADDR(state->cmd_end_debug_utils_label, vkCmdEndDebugUtilsLabelEXT);
    
    VkDebugUtilsMessengerCreateInfoEXT debug_info = {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .pNext = NULL,
        .flags = 0,
        .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
        .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
        .pfnUserCallback = debug_messenger_callback
    };

    result = create_debug_utils_messenger(state->instance, &debug_info, state->allocation_callbacks, &state->debug_messenger);
    if (result != VK_SUCCESS) {
        LOG_ERROR("Failed to create vulkan debug messenger: %s", vk_result_to_string(result));
        return FALSE;
    }

    LOG_INFO("Vulkan debug messenger created");

    return TRUE;
}

#undef LOG_SCOPE
#define LOG_SCOPE "VULKAN DEBUG"

static VkBool32 VKAPI_PTR debug_messenger_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageTypes,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
    void *pUserData) {
    switch (messageSeverity) {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
        LOG_TRACE("%s", pCallbackData->pMessage);
        break;

    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
        LOG_INFO("%s", pCallbackData->pMessage);
        break;

    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        LOG_WARN("%s", pCallbackData->pMessage);
        break;

    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        LOG_ERROR("%s", pCallbackData->pMessage);
        break;
    }

    return VK_FALSE;
}