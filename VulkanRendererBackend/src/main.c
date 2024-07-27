#include "renderer_backend.h"
#include <core/memory.h>
#include <core/plugins.h>

#define LOG_SCOPE "VULKAN BACKEND"
#include <core/log.h>

static b8 plugin_init(void **state);
static void plugin_deinit(void *state);

plugin_interface _plugin_interface = {
    .init = plugin_init,
    .deinit = plugin_deinit,
    .state = NULL,
};

b8 plugin_init(void **state) {
    renderer_backend_interface *interface = mem_alloc(MEMORY_TAG_RENDERER, sizeof(renderer_backend_interface));
    *state = interface;
    mem_zero(interface, sizeof(renderer_backend_interface));

    interface->init = vulkan_init;
    interface->deinit = vulkan_deinit;
    interface->frame_prepare = vulkan_frame_prepare;
    interface->command_list_begin = vulkan_command_list_begin;
    interface->command_list_end = vulkan_command_list_end;
    interface->frame_render = vulkan_frame_render;

    return TRUE;
}

void plugin_deinit(void *state) {
    mem_free(state);

    LOG_INFO("Vulkan renderer backend deinitialized");
    return;
}
