#include <core/plugins.h>
#include <core/memory.h>
#include "renderer_backend.h"

#define LOG_SCOPE "VULKAN BACKEND"
#include <core/log.h>

static b8 plugin_init(void **state);
static void plugin_deinit(void *state);

plugin_interface _plugin_interface = {
    .init = plugin_init,
    .deinit = plugin_deinit,
    .state = NULL
};

b8 plugin_init(void **state) {
    renderer_backend_interface *interface = mem_alloc(MEMORY_TAG_RENDERER, sizeof(renderer_backend_interface));
    *state = interface;
    mem_zero(interface, sizeof(renderer_backend_interface));

    interface->init = vulkan_init;
    interface->deinit = vulkan_deinit;

    return TRUE;
}

void plugin_deinit(void *state) {
    mem_free(state);

    LOG_INFO("Vulkan renderer backend deinitialized");
    return;
}