#pragma once

#include "xdg-shell-client-protocol.h"
#include <common.h>

/**
 * @brief Called when a window is closed.
 *
 * @param[in] data The user data
 * @param[in] wl_toplevel The toplevel
 */
void wayland_toplevel_handle_close(void *data, struct xdg_toplevel *wl_toplevel);

/**
 * @brief Called when a window is configured.
 *
 * @param[in] data The user data
 * @param[in] wl_toplevel The toplevel
 * @param[in] width The width of the window
 * @param[in] height The height of the window
 * @param[in] states The states
 */
void wayland_toplevel_handle_configure(
    void *data, struct xdg_toplevel *wl_toplevel, i32 width, i32 height, struct wl_array *states);

/**
 * @brief Called when the capabilities of the window manager change.
 *
 * @param[in] data The user data
 * @param[in] wl_toplevel The toplevel
 * @param[in] wm_capabilities The capabilities
 */
void wayland_toplevel_handle_wm_capabilities(void *data, struct xdg_toplevel *wl_toplevel, struct wl_array *wm_capabilities);

/**
 * @brief Called when the bounds of the window change.
 *
 * @param[in] data The user data
 * @param[in] wl_toplevel The toplevel
 * @param[in] width The width of the window
 * @param[in] height The height of the window
 */
void wayland_toplevel_handle_configure_bounds(void *data, struct xdg_toplevel *wl_toplevel, i32 width, i32 height);
