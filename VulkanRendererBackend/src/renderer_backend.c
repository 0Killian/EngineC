#include "renderer_backend.h"
#include "internal_types.h"
#include "platform/vulkan_platform.h"
#include "vulkan_command_buffer.h"
#include "vulkan_device.h"
#include "vulkan_swapchain.h"
#include <core/event.h>
#include <core/memory.h>
#include <vulkan/vulkan.h>

#define LOG_SCOPE "VULKAN RENDERER BACKEND"
#include "vulkan_utils.h"

#define GET_PROC_ADDR(var, name)                                     \
    var = (PFN_##name)vkGetInstanceProcAddr(state->instance, #name); \
    if (var == NULL) {                                               \
        LOG_ERROR("Failed to get " #name " function address");       \
        return FALSE;                                                \
    }

static b8 create_instance(vulkan_state *state, renderer_backend_config *config);
static b8 debug_setup(vulkan_state *state);

static VkBool32 VKAPI_PTR debug_messenger_callback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                   VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                                                   const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                                   void *pUserData);

static b8 recreate_frame_resources(vulkan_state *state, u32 current_frame_count) {
    if (current_frame_count != 0) {
        for (u32 i = 0; i < current_frame_count; i++) {
            vkWaitForFences(state->device.logical_device, 1, &state->in_flight_fences[i], VK_TRUE, UINT64_MAX);

            if (state->render_finished_semaphores[i] != VK_NULL_HANDLE) {
                vkDestroySemaphore(state->device.logical_device,
                                   state->render_finished_semaphores[i],
                                   state->allocation_callbacks);
            }

            if (state->image_available_semaphores[i] != VK_NULL_HANDLE) {
                vkDestroySemaphore(state->device.logical_device,
                                   state->image_available_semaphores[i],
                                   state->allocation_callbacks);
            }

            if (state->in_flight_fences[i] != VK_NULL_HANDLE) {
                vkDestroyFence(state->device.logical_device, state->in_flight_fences[i], state->allocation_callbacks);
            }

            vulkan_command_buffer_free(state, state->device.graphics_command_pool, &state->command_buffers[i]);
        }

        mem_free(state->image_available_semaphores);
        mem_free(state->render_finished_semaphores);
        mem_free(state->in_flight_fences);
        mem_free(state->command_buffers);
    }

    if (state->swapchain.max_frames_in_flight == 0) {
        return TRUE;
    }

    state->image_available_semaphores = mem_alloc(MEMORY_TAG_RENDERER, state->swapchain.max_frames_in_flight * sizeof(VkSemaphore));
    state->render_finished_semaphores = mem_alloc(MEMORY_TAG_RENDERER, state->swapchain.max_frames_in_flight * sizeof(VkSemaphore));
    state->in_flight_fences = mem_alloc(MEMORY_TAG_RENDERER, state->swapchain.max_frames_in_flight * sizeof(VkFence));
    state->command_buffers = mem_alloc(MEMORY_TAG_RENDERER, state->swapchain.max_frames_in_flight * sizeof(vulkan_command_buffer));

    mem_zero(state->image_available_semaphores, sizeof(VkSemaphore) * state->swapchain.max_frames_in_flight);
    mem_zero(state->render_finished_semaphores, sizeof(VkSemaphore) * state->swapchain.max_frames_in_flight);
    mem_zero(state->in_flight_fences, sizeof(VkFence) * state->swapchain.max_frames_in_flight);
    mem_zero(state->command_buffers, sizeof(vulkan_command_buffer) * state->swapchain.max_frames_in_flight);

    for (u32 i = 0; i < state->swapchain.max_frames_in_flight; i++) {
        char ia_semaphore_name[] = "FrameImageAvailable0";
        ia_semaphore_name[20] = '0' + i;

        char rf_semaphore_name[] = "FrameRenderFinished0";
        rf_semaphore_name[20] = '0' + i;

        char buffer_name[] = "Frame0";
        buffer_name[5] = '0' + i;

        VkSemaphoreCreateInfo semaphore_create_info = {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            .pNext = NULL,
            .flags = 0,
        };

        VkFenceCreateInfo fence_create_info = {
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .pNext = NULL,
            .flags = VK_FENCE_CREATE_SIGNALED_BIT,
        };

        VkResult result = vkCreateSemaphore(state->device.logical_device, &semaphore_create_info, state->allocation_callbacks, &state->image_available_semaphores[i]);
        if (result != VK_SUCCESS) {
            LOG_ERROR("Failed to create image available semaphore");
            goto destroy;
        }

        result = vkCreateSemaphore(state->device.logical_device, &semaphore_create_info, state->allocation_callbacks, &state->render_finished_semaphores[i]);
        if (result != VK_SUCCESS) {
            LOG_ERROR("Failed to create render finished semaphore");
            goto destroy;
        }

        result = vkCreateFence(state->device.logical_device, &fence_create_info, state->allocation_callbacks, &state->in_flight_fences[i]);
        if (result != VK_SUCCESS) {
            LOG_ERROR("Failed to create in flight fence");
            goto destroy;
        }

        VK_SET_OBJECT_DEBUG_NAME(state, VK_OBJECT_TYPE_SEMAPHORE, state->image_available_semaphores[i], "Semaphore.", ia_semaphore_name);
        VK_SET_OBJECT_DEBUG_NAME(state, VK_OBJECT_TYPE_SEMAPHORE, state->render_finished_semaphores[i], "Semaphore.", rf_semaphore_name);
        VK_SET_OBJECT_DEBUG_NAME(state, VK_OBJECT_TYPE_FENCE, state->in_flight_fences[i], "Fence.", buffer_name);

        if (!vulkan_command_buffer_alloc(state, state->device.graphics_command_pool, buffer_name, TRUE, &state->command_buffers[i])) {
            LOG_ERROR("Failed to allocate command buffer");
            goto destroy;
        }
    }

    return TRUE;

destroy:
    for (u32 i = 0; i < state->swapchain.max_frames_in_flight; i++) {
        if (state->render_finished_semaphores[i] != VK_NULL_HANDLE) {
            vkDestroySemaphore(state->device.logical_device,
                               state->render_finished_semaphores[i],
                               state->allocation_callbacks);
        }

        if (state->image_available_semaphores[i] != VK_NULL_HANDLE) {
            vkDestroySemaphore(state->device.logical_device,
                               state->image_available_semaphores[i],
                               state->allocation_callbacks);
        }

        if (state->in_flight_fences[i] != VK_NULL_HANDLE) {
            vkDestroyFence(state->device.logical_device, state->in_flight_fences[i], state->allocation_callbacks);
        }

        if (state->command_buffers[i].handle != VK_NULL_HANDLE) {
            vulkan_command_buffer_free(state, state->device.graphics_command_pool, &state->command_buffers[i]);
        }
    }

    mem_free(state->image_available_semaphores);
    mem_free(state->render_finished_semaphores);
    mem_free(state->in_flight_fences);
    mem_free(state->command_buffers);

    return FALSE;
}

static void on_window_resize(event_type type, event_data data, void *user_data) {
    vulkan_state *state = (vulkan_state *)user_data;

    if (data.vec2f.x > 0.0f && data.vec2f.y > 0.0f) {
        u32 frame_in_flights = state->swapchain.max_frames_in_flight;
        vulkan_swapchain_recreate(state, data.vec2f.x, data.vec2f.y, &state->swapchain);

        if (frame_in_flights != state->swapchain.max_frames_in_flight) {
            if (!recreate_frame_resources(state, frame_in_flights)) {
                LOG_ERROR("Failed to recreate frame resources");
                // TODO: Signal error
            }
        }
    }

    return;
}

b8 vulkan_init(renderer_backend_interface *interface, renderer_backend_config *config, const window *window) {
    vulkan_state *state = mem_alloc(MEMORY_TAG_RENDERER, sizeof(vulkan_state));
    mem_zero(state, sizeof(vulkan_state));
    interface->internal_data = state;

    state->win = window;

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

    if (!vulkan_device_select(state)) {
        LOG_ERROR("Failed to select vulkan device");
        return FALSE;
    }

    if (!vulkan_device_create(state)) {
        LOG_ERROR("Failed to create vulkan device");
        return FALSE;
    }

    LOG_INFO("Vulkan renderer backend initialized");

    if (!vulkan_swapchain_create(state, window->width, window->height, &state->swapchain)) {
        LOG_ERROR("Failed to create vulkan swapchain");
        return FALSE;
    }

    if (!recreate_frame_resources(state, 0)) {
        LOG_ERROR("Failed to recreate frame resources");
        return FALSE;
    }

    if (!event_register_callback(EVENT_TYPE_WINDOW_RESIZED, on_window_resize, state, &state->on_resize_handler)) {
        LOG_ERROR("Failed to register window resize callback");
        return FALSE;
    }

    return TRUE;
}

void vulkan_deinit(renderer_backend_interface *interface) {
    vulkan_state *state = (vulkan_state *)interface->internal_data;

    if (state == NULL) {
        LOG_ERROR("Vulkan renderer backend not initialized");
        return;
    }

    if (state->on_resize_handler != INVALID_UUID) {
        event_unregister_callback(EVENT_TYPE_WINDOW_RESIZED, state->on_resize_handler);
        state->on_resize_handler = INVALID_UUID;
    }

    // NOTE: when max_frame_in_flight is 0, the frame resources will not be recreated and juste destroyed.
    u32 frame_count = state->swapchain.max_frames_in_flight;
    state->swapchain.max_frames_in_flight = 0;
    recreate_frame_resources(state, frame_count);

    vulkan_swapchain_destroy(state, &state->swapchain);
    vulkan_device_destroy(state);

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
        .apiVersion = VK_API_VERSION_1_3,
    };

#ifdef DEBUG
    uint32_t layer_count = 1;
    const char *layer_names[] = { "VK_LAYER_KHRONOS_validation" };
#else
    uint32_t layer_count = 0;
    const char **layer_names = NULL;
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
        .ppEnabledExtensionNames = extensions.data,
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
        .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
        .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
        .pfnUserCallback = debug_messenger_callback,
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

static VkBool32 VKAPI_PTR debug_messenger_callback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                   VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                                                   const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                                   void *pUserData) {
    switch (messageSeverity) {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT: LOG_TRACE("%s", pCallbackData->pMessage); break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT: LOG_INFO("%s", pCallbackData->pMessage); break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: LOG_WARN("%s", pCallbackData->pMessage); break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT: LOG_ERROR("%s", pCallbackData->pMessage); break;
    default: return VK_FALSE;
    }

    return VK_FALSE;
}
