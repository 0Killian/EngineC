#pragma once

#include "xdg-shell-client-protocol.h"
#include <common.h>
#include <wayland-client.h>

/**
 * @brief Called when the surface enters a new output.
 *
 * @param[in] data The user data
 * @param[in] wl_surface The surface
 * @param[in] output The new output
 */ 
void wayland_surface_handle_enter(void *data, struct wl_surface *wl_surface, struct wl_output *output);

/**
 * @brief Called when the surface leaves an output.
 *
 * @param[in] data The user data
 * @param[in] wl_surface The surface
 * @param[in] output The old output
 */
void wayland_surface_handle_leave(void *data, struct wl_surface *wl_surface, struct wl_output *output);

/**
 * @brief Called to tell the buffer preferred scale
 *
 * @param[in] data The user data
 * @param[in] wl_surface The surface
 * @param[in] scale The preferred scale
 */
void wayland_surface_handle_preferred_buffer_scale(void *data, struct wl_surface *wl_surface, i32 scale);

/**
 * @brief Called to tell the buffer preferred transform
 *
 * @param[in] data The user data
 * @param[in] wl_surface The surface
 * @param[in] transform The preferred transform
 */
void wayland_surface_handle_preferred_buffer_transform(void *data, struct wl_surface *wl_surface, u32 transform);

/**
 * @brief Called when the XDG surface configures itself
 *
 * @param[in] data The user data
 * @param[in] xdg_surface The surface
 * @param[in] serial The serial of the event
 */
void wayland_xdg_surface_handle_configure(void *data, struct xdg_surface *xdg_surface, u32 serial);
