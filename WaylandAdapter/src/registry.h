#pragma once

#include <common.h>
#include <wayland-client.h>

/**
 * @brief Called when the registry registers a new global.
 *
 * @param[in] data The user data
 * @param[in] wl_registry The registry object.
 * @param[in] name The name of the global.
 * @param[in] interface The interface of the global.
 * @param[in] version The version of the global.
 */
void wayland_registry_handle_global(void *data, struct wl_registry *registry, u32 name, const char *interface, u32 version);

/**
 * @brief Called when the registry unregisters a global.
 *
 * @param[in] data The user data
 * @param[in] wl_registry The registry object.
 * @param[in] name The name of the global.
 */
void wayland_registry_handle_global_remove(void *data, struct wl_registry *registry, u32 name);
