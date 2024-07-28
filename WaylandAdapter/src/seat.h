#pragma once

#include <common.h>
#include <wayland-client.h>

/**
 * @brief Called when the name of the seat changes.
 *
 * @param[in] data The user data
 * @param[in] wl_seat The seat
 * @param[in] name The new name
 */
void wayland_seat_handle_name(void *data, struct wl_seat *wl_seat, const char *name);

/**
 * @brief Called when the capabilities of the seat changes.
 *
 * @param[in] data The user data
 * @param[in] wl_seat The seat
 * @param[in] capabilities The new capabilities
 */
void wayland_seat_handle_capabilities(void *data, struct wl_seat *wl_seat, u32 capabilities);
