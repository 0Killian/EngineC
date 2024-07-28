#pragma once

#include <common.h>
#include <wayland-client.h>

/**
 * @brief Called when a surface takes the focus of the pointer.
 *
 * @param[in] data The user data
 * @param[in] wl_pointer The pointer
 * @param[in] serial The serial of the event
 * @param[in] surface The surface that took the focus
 * @param[in] sx The x coordinate of the pointer
 * @param[in] sy The y coordinate of the pointer
 */
void wayland_pointer_handle_enter(
    void *data, struct wl_pointer *wl_pointer, uint32_t serial, struct wl_surface *surface, wl_fixed_t sx, wl_fixed_t sy);

/**
 * @brief Called when a surface loses the focus of the pointer.
 *
 * @param[in] data The user data
 * @param[in] wl_pointer The pointer
 * @param[in] serial The serial of the event
 * @param[in] surface The surface that lost the focus
 */
void wayland_pointer_handle_leave(void *data, struct wl_pointer *wl_pointer, uint32_t serial, struct wl_surface *surface);

/**
 * @brief Called when the pointer is moved.
 *
 * @param[in] data The user data
 * @param[in] wl_pointer The pointer
 * @param[in] time The time of the event
 * @param[in] sx The x coordinate of the pointer
 * @param[in] sy The y coordinate of the pointer
 */
void wayland_pointer_handle_motion(void *data, struct wl_pointer *wl_pointer, uint32_t time, wl_fixed_t sx, wl_fixed_t sy);

/**
 * @brief Called when the pointer is clicked.
 *
 * @param[in] data The user data
 * @param[in] wl_pointer The pointer
 * @param[in] serial The serial of the event
 * @param[in] time The time of the event
 * @param[in] button The button that was clicked
 * @param[in] state The state of the button
 */
void wayland_pointer_handle_button(
    void *data, struct wl_pointer *wl_pointer, uint32_t serial, uint32_t time, uint32_t button, uint32_t state);

/**
 * @brief Called when the pointer is scrolled.
 *
 * @param[in] data The user data
 * @param[in] wl_pointer The pointer
 * @param[in] time The time of the event
 * @param[in] axis The axis that was scrolled
 * @param[in] value The value of the axis
 */
void wayland_pointer_handle_axis(void *data, struct wl_pointer *wl_pointer, uint32_t time, uint32_t axis, wl_fixed_t value);

/**
 * @brief Called when an input frame is complete
 *
 * @param[in] data The user data
 * @param[in] wl_pointer The pointer
 */
void wayland_pointer_handle_frame(void *data, struct wl_pointer *wl_pointer);

// ???
void wayland_pointer_handle_axis_source(void *data, struct wl_pointer *wl_pointer, uint32_t axis_source);
void wayland_pointer_handle_axis_stop(void *data, struct wl_pointer *wl_pointer, uint32_t time, uint32_t axis);
void wayland_pointer_handle_axis_discrete(void *data, struct wl_pointer *wl_pointer, uint32_t axis, int32_t discrete);
void wayland_pointer_handle_axis_value120(void *data, struct wl_pointer *wl_pointer, uint32_t axis, int32_t value120);
void wayland_pointer_handle_axis_relative_direction(void *data, struct wl_pointer *wl_pointer, uint32_t axis, uint32_t direction);
