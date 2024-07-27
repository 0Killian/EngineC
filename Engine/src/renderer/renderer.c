#include "renderer/renderer.h"
#include "core/plugins.h"
#include "core/memory.h"
#include "renderer/renderer_backend_interface.h"

#define LOG_SCOPE "RENDERER SYSTEM"
#include "core/log.h"

typedef struct renderer_system_state {
    plugin backend_plugin;
} renderer_system_state;

static renderer_system_state *state = NULL;

/**
 * @brief Initializes the renderer system.
 * 
 * Should be called twice, once to get the required allocation size (with state == NULL), and a second time to actually
 * initialize the renderer system (with state != NULL).
 * 
 * @param[in] state A pointer to a memory region to store the state of the renderer system. To obtain the needed size, pass NULL.
 * @param[out] size_requirement A pointer to the size of the memory that should be allocated.
 * @param[in] window A pointer to the window to use for the renderer.
 * 
 * @retval TRUE Success
 * @retval FALSE Failure
 */
b8 renderer_init(void *state_storage, u64 *size_requirement, const window *window) {
    if (state_storage == NULL) {
        *size_requirement = sizeof(renderer_system_state);
        return TRUE;
    }

    state = (renderer_system_state*)state_storage;
    mem_zero(state, sizeof(renderer_system_state));

    if (!plugins_load("VulkanRendererBackend", &state->backend_plugin)) {
        LOG_ERROR("Failed to load vulkan renderer backend");
        return FALSE;
    }

    renderer_backend_interface *interface = (renderer_backend_interface*)state->backend_plugin.interface.state;
    renderer_backend_config config = { "Vulkan Renderer" };

    if (!interface->init(interface, &config, window)) {
        LOG_ERROR("Failed to initialize vulkan renderer backend");
        return FALSE;
    }

    return TRUE;
}

/**
 * @brief Deinitializes the renderer system.
 * 
 * @param[in] state A pointer to the state of the renderer system.
 */
void renderer_deinit(void *) {
    renderer_backend_interface *interface = (renderer_backend_interface*)state->backend_plugin.interface.state;

    interface->deinit(interface);

    plugins_unload("VulkanRendererBackend");

    state = NULL;
}

/**
 * @brief Prepares a frame for rendering. This call must be followed by a call to @ref renderer_frame_render after rendering
 * is done.
 * 
 * @param[in] state A pointer to the state of the renderer system.
 * @param[in,out] packet A pointer to the frame packet to render.
 *
 * @retval TRUE Success
 * @retval FALSE Failure
 */
b8 renderer_frame_prepare(void *_, frame_packet *packet) {
    renderer_backend_interface *interface = (renderer_backend_interface*)state->backend_plugin.interface.state;
    
    // TODO: Setup the high level frame

    return interface->frame_prepare(interface, packet);
}

/**
 * @brief Begins a command list. This call must be followed by a call to @ref renderer_command_list_end after recording render
 * calls.
 *
 * @param[in] state A pointer to the state of the renderer system.
 * @param[in,out] packet A pointer to the frame packet to render.
 *
 * @retval TRUE Success
 * @retval FALSE Failure
 */
b8 renderer_command_list_begin(void *_, frame_packet *packet) {
    renderer_backend_interface *interface = (renderer_backend_interface*)state->backend_plugin.interface.state;
    return interface->command_list_begin(interface, packet);
}

/**
 * @brief Ends a command list. This call must be preceded by a call to @ref renderer_command_list_begin before recording render
 * calls.
 *
 * @param[in] state A pointer to the state of the renderer system.
 * @param[in,out] packet A pointer to the frame packet to render.
 *
 * @retval TRUE Success
 * @retval FALSE Failure
 */
b8 renderer_command_list_end(void *_, frame_packet *packet) {
    renderer_backend_interface *interface = (renderer_backend_interface*)state->backend_plugin.interface.state;
    return interface->command_list_end(interface, packet);
}

/**
 * @brief Renders the current frame. This call must be preceded by a call to @ref renderer_frame_prepare before rendering
 * is done.
 * 
 * @param[in] state A pointer to the state of the renderer system.
 * @param[in,out] packet A pointer to the frame packet to render.
 */
b8 renderer_frame_render(void *_, frame_packet *packet) {
    renderer_backend_interface *interface = (renderer_backend_interface*)state->backend_plugin.interface.state;

    // TODO: Render the high level frame

    return interface->frame_render(interface, packet);
}
