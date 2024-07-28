#pragma once

#include <common.h>
#include <wayland-client.h>

/**
 * @brief Called when a keymap is received.
 *
 * @param[in] data The user data
 * @param[in] wl_keyboard The keyboard object.
 * @param[in] format The format of the keymap.
 * @param[in] fd The file descriptor of the keymap.
 * @param[in] size The size of the keymap.
 */
void wayland_keyboard_handle_keymap(void *data, struct wl_keyboard *wl_keyboard, u32 format, i32 fd, u32 size);

/**
 * @brief Called when the provided surface takes focus of the keyboard.
 *
 * @param[in] data The user data
 * @param[in] wl_keyboard The keyboard object.
 * @param[in] serial The serial of the enter event.
 * @param[in] surface The surface that takes focus of the keyboard.
 * @param[in] keys The keys that are currently pressed.
 */
void wayland_keyboard_handle_enter(
    void *data, struct wl_keyboard *wl_keyboard, u32 serial, struct wl_surface *surface, struct wl_array *keys);

/**
 * @brief Called when the provided surface loses focus of the keyboard.
 *
 * @param[in] data The user data
 * @param[in] wl_keyboard The keyboard object.
 * @param[in] serial The serial of the leave event.
 * @param[in] surface The surface that loses focus of the keyboard.
 */
void wayland_keyboard_handle_leave(void *data, struct wl_keyboard *wl_keyboard, u32 serial, struct wl_surface *surface);

/**
 * @brief Called when a key is pressed or released.
 *
 * @param[in] data The user data
 * @param[in] wl_keyboard The keyboard object.
 * @param[in] serial The serial of the key event.
 * @param[in] time The time of the event.
 * @param[in] key The key of the event.
 * @param[in] state The state of the event.
 */
void wayland_keyboard_handle_key(void *data, struct wl_keyboard *wl_keyboard, u32 serial, u32 time, u32 key, u32 state);

/**
 * @brief Called when the keyboard modifiers are changed.
 *
 * @param[in] data The user data
 * @param[in] wl_keyboard The keyboard object.
 * @param[in] serial The serial of the modifiers event.
 * @param[in] mods_depressed The depressed modifiers.
 * @param[in] mods_latched The latched modifiers.
 * @param[in] mods_locked The locked modifiers.
 * @param[in] group The group of the modifiers.
 */
void wayland_keyboard_handle_modifiers(
    void *data, struct wl_keyboard *wl_keyboard, u32 serial, u32 mods_depressed, u32 mods_latched, u32 mods_locked, u32 group);

/**
 * @brief Called when the keyboard repeat info is changed.
 *
 * @param[in] data The user data
 * @param[in] wl_keyboard The keyboard object.
 * @param[in] rate The rate of the repeat.
 * @param[in] delay The delay of the repeat.
 */
void wayland_keyboard_handle_repeat_info(void *data, struct wl_keyboard *wl_keyboard, i32 rate, i32 delay);
