#include "pointer.h"
#include "wayland_adapter.h"
#include <platform/linux_adapter.h>
#include <core/event.h>

void wayland_pointer_handle_enter(void *data, struct wl_pointer *wl_pointer, uint32_t serial, struct wl_surface *surface, wl_fixed_t sx, wl_fixed_t sy) {
    (void)wl_pointer;
    (void)serial;
    linux_adapter *adapter = (linux_adapter *)data;

    for (uuid i = 0; i < adapter->platform_state->windows.count; i++) {
        window *window = adapter->platform_state->windows.data[i];
        if (window->platform_state->surface == surface) {
            adapter->adapter_state->pointer_focus = i;
        }
    }

    event_data event = {
        .vec2f = {
            .x = wl_fixed_to_double(sx),
            .y = wl_fixed_to_double(sy)
        }
    };

    event_fire(EVENT_TYPE_MOUSE_MOVED, event);
}

void wayland_pointer_handle_leave(void *data, struct wl_pointer *wl_pointer, uint32_t serial, struct wl_surface *surface) {
    (void)wl_pointer;
    (void)serial;
    (void)surface;
    linux_adapter *adapter = (linux_adapter *)data;

    adapter->adapter_state->pointer_focus = INVALID_UUID;
}

void wayland_pointer_handle_motion(void *data, struct wl_pointer *wl_pointer, uint32_t time, wl_fixed_t sx, wl_fixed_t sy) {
    (void)wl_pointer;
    (void)time;
    linux_adapter *adapter = (linux_adapter *)data;

    if (adapter->adapter_state->pointer_focus != INVALID_UUID) {
        event_data event = {
            .vec2f = {
                .x = wl_fixed_to_double(sx),
                .y = wl_fixed_to_double(sy)
            }
        };

        event_fire(EVENT_TYPE_MOUSE_MOVED, event);
    }
}

void wayland_pointer_handle_button(void *data, struct wl_pointer *wl_pointer, uint32_t serial, uint32_t time, uint32_t button, uint32_t state) {
    (void)wl_pointer;
    (void)serial;
    (void)time;
    linux_adapter *adapter = (linux_adapter *)data;

    if (adapter->adapter_state->pointer_focus != INVALID_UUID) {
        event_data event = { .u32 = button - 0x110 }; // button - BTN_MOUSE

        event_fire(state == WL_POINTER_BUTTON_STATE_PRESSED ? EVENT_TYPE_MOUSE_BUTTON_PRESSED : EVENT_TYPE_MOUSE_BUTTON_RELEASED, event);
    }
}

void wayland_pointer_handle_axis(void *data, struct wl_pointer *wl_pointer, uint32_t time, uint32_t axis, wl_fixed_t value) {
    (void)wl_pointer;
    (void)time;
    linux_adapter *adapter = (linux_adapter *)data;

    if (adapter->adapter_state->pointer_focus != INVALID_UUID) {
        if (axis == WL_POINTER_AXIS_VERTICAL_SCROLL) {
            event_data event = {
                .f32 = wl_fixed_to_double(value)
            };

            event_fire(EVENT_TYPE_MOUSE_WHEEL, event);
        }
    }
}

void wayland_pointer_handle_frame(void *data, struct wl_pointer *wl_pointer) {
    (void)data;
    (void)wl_pointer;
}

void wayland_pointer_handle_axis_source(void *data, struct wl_pointer *wl_pointer, uint32_t axis_source) {
    (void)data;
    (void)wl_pointer;
    (void)axis_source;
}

void wayland_pointer_handle_axis_stop(void *data, struct wl_pointer *wl_pointer, uint32_t time, uint32_t axis) {
    (void)data;
    (void)wl_pointer;
    (void)time;
    (void)axis;
}

void wayland_pointer_handle_axis_discrete(void *data, struct wl_pointer *wl_pointer, uint32_t axis, int32_t discrete) {
    (void)data;
    (void)wl_pointer;
    (void)axis;
    (void)discrete;
}

void wayland_pointer_handle_axis_value120(void *data, struct wl_pointer *wl_pointer, uint32_t axis, int32_t value120) {
    (void)data;
    (void)wl_pointer;
    (void)axis;
    (void)value120;
}

void wayland_pointer_handle_axis_relative_direction(void *data, struct wl_pointer *wl_pointer, uint32_t axis, uint32_t direction) {
    (void)data;
    (void)wl_pointer;
    (void)axis;
    (void)direction;
}
