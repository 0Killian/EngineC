#pragma once

#include <common.h>
#include <wayland-client.h>

void wayland_seat_handle_name(void *data, struct wl_seat *wl_seat, const char *name);
void wayland_seat_handle_capabilities(void *data, struct wl_seat *wl_seat, u32 capabilities);
