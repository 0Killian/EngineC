#pragma once

#include <common.h>
#include <wayland-client.h>
#include "xdg-shell-client-protocol.h"

void wayland_surface_handle_enter(void *data, struct wl_surface *wl_surface, struct wl_output *output);
void wayland_surface_handle_leave(void *data, struct wl_surface *wl_surface, struct wl_output *output);
void wayland_surface_handle_preferred_buffer_scale(void *data, struct wl_surface *wl_surface, i32 scale);
void wayland_surface_handle_preferred_buffer_transform(void *data, struct wl_surface *wl_surface, u32 transform);
void wayland_xdg_surface_handle_configure(void *data, struct xdg_surface *xdg_surface, u32 serial);
