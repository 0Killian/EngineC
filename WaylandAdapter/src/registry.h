#pragma once

#include <common.h>
#include <wayland-client.h>

void wayland_registry_handle_global(void *data, struct wl_registry *registry, u32 name, const char *interface, u32 version);
void wayland_registry_handle_global_remove(void *data, struct wl_registry *registry, u32 name);
