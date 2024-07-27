#pragma once

#include <common.h>
#include "xdg-shell-client-protocol.h"

void wayland_toplevel_handle_close(void *data, struct xdg_toplevel *wl_toplevel);
void wayland_toplevel_handle_configure(void *data, struct xdg_toplevel *wl_toplevel, i32 width, i32 height, struct wl_array *states);
void wayland_toplevel_handle_wm_capabilities(void *data, struct xdg_toplevel *wl_toplevel, struct wl_array *wm_capabilities);
void wayland_toplevel_handle_configure_bounds(void *data, struct xdg_toplevel *wl_toplevel, i32 width, i32 height);
