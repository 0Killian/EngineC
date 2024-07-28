/**
 * @file platform_linux_internal.h
 * @author Killian Bellouard (killianbellouard@gmail.com)
 * @brief This file defines the internals of the linux platform layer, to be included to wayland extension code.
 * @version 0.1
 * @date 2024-07-22
 */

#pragma once

#include "xdg-decoration-client-protocol.h"
#include "xdg-shell-client-protocol.h"
#include <common.h>
#include <libdecor-0/libdecor.h>
#include <wayland-client.h>

/** @brief The state of the wayland linux adapter */
struct linux_adapter_state {
    struct wl_display *display;
    struct wl_registry *registry;
    struct wl_compositor *compositor;
    struct wl_seat *seat;
    struct zxdg_decoration_manager_v1 *decoration_manager;
    struct xdg_wm_base *shell;

    struct wl_pointer *pointer;
    struct wl_keyboard *keyboard;

    struct wl_registry_listener registry_listener;
    struct wl_seat_listener seat_listener;
    struct wl_pointer_listener pointer_listener;
    struct wl_keyboard_listener keyboard_listener;
    struct xdg_wm_base_listener shell_listener;
    struct wl_surface_listener surface_listener;
    struct xdg_surface_listener xdg_surface_listener;
    struct xdg_toplevel_listener toplevel_listener;
    struct zxdg_toplevel_decoration_v1_listener decoration_listener;

    struct libdecor_interface libdecor_iface;
    struct libdecor_frame_interface libdecor_frame_iface;

    struct libdecor *decorator;

    uuid pointer_focus;
    uuid keyboard_focus;
};

/** @brief The window state of the wayland linux adapter */
struct window_platform_state {
    struct linux_adapter *adapter;
    struct wl_surface *surface;

    struct xdg_surface *xdg_surface;
    struct xdg_toplevel *toplevel;
    struct zxdg_toplevel_decoration_v1 *decoration;

    b8 client_side_decorations;
    struct libdecor_frame *frame;
    enum libdecor_window_state state;
    i32 floating_width;
    i32 floating_height;
    b8 first_resize;
};
