#pragma once

#include <common.h>
#include <wayland-client.h>

void wayland_keyboard_handle_keymap(void *data, struct wl_keyboard *wl_keyboard, u32 format, i32 fd, u32 size);
void wayland_keyboard_handle_enter(void *data, struct wl_keyboard *wl_keyboard, u32 serial, struct wl_surface *surface, struct wl_array *keys);
void wayland_keyboard_handle_leave(void *data, struct wl_keyboard *wl_keyboard, u32 serial, struct wl_surface *surface);
void wayland_keyboard_handle_key(void *data, struct wl_keyboard *wl_keyboard, u32 serial, u32 time, u32 key, u32 state);
void wayland_keyboard_handle_modifiers(void *data, struct wl_keyboard *wl_keyboard, u32 serial, u32 mods_depressed, u32 mods_latched, u32 mods_locked, u32 group);
void wayland_keyboard_handle_repeat_info(void *data, struct wl_keyboard *wl_keyboard, i32 rate, i32 delay);
