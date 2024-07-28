#include "toplevel.h"
#include "wayland_adapter.h"
#include <core/event.h>
#include <platform/linux_adapter.h>

void wayland_toplevel_handle_close(void *data, struct xdg_toplevel *wl_toplevel) {
    linux_adapter *adapter = ((window *)data)->platform_state->adapter;

    if (adapter->platform_state->window_closed_callback != NULL) {
        adapter->platform_state->window_closed_callback(data);
    }
}

void wayland_toplevel_handle_configure(
    void *data, struct xdg_toplevel *wl_toplevel, i32 width, i32 height, struct wl_array *states) {
    (void)wl_toplevel;
    linux_adapter *adapter = ((window *)data)->platform_state->adapter;

    if (((window *)data)->platform_state->first_resize) {
        ((window *)data)->platform_state->first_resize = FALSE;
    } else {
        event_data event = {
            .vec2f = { .x = width, .y = height }
        };

        event_fire(EVENT_TYPE_WINDOW_RESIZED, event);
    }
}

void wayland_toplevel_handle_wm_capabilities(void *data, struct xdg_toplevel *wl_toplevel, struct wl_array *wm_capabilities) {
    (void)data;
    (void)wl_toplevel;
    (void)wm_capabilities;
}

void wayland_toplevel_handle_configure_bounds(void *data, struct xdg_toplevel *wl_toplevel, i32 width, i32 height) {
    (void)data;
    (void)wl_toplevel;
    (void)width;
    (void)height;
}
