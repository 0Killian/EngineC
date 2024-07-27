#include "decoration.h"
#include "wayland_adapter.h"
#include <libdecor-0/libdecor.h>
#include <platform/linux_adapter.h>
#include <core/event.h>

#define LOG_SCOPE "WAYLAND ADAPTER"
#include <core/log.h>

void wayland_decoration_handle_configure_ssd(void *data, struct zxdg_toplevel_decoration_v1 *decoration, u32 mode) {
    window *window = data;

    if (mode == ZXDG_TOPLEVEL_DECORATION_V1_MODE_CLIENT_SIDE) {
        LOG_WARN("Server side decorations not supported, falling back to client side decorations");
        zxdg_toplevel_decoration_v1_set_mode(decoration, ZXDG_TOPLEVEL_DECORATION_V1_MODE_CLIENT_SIDE);
        zxdg_toplevel_decoration_v1_destroy(decoration);

        if (window->platform_state->toplevel != NULL) {
            xdg_toplevel_destroy(window->platform_state->toplevel);
        }

        if (window->platform_state->xdg_surface != NULL) {
            xdg_surface_destroy(window->platform_state->xdg_surface);
        }

        window->platform_state->toplevel = NULL;
        window->platform_state->xdg_surface = NULL;

        wayland_setup_csd(data);
    }
}

void wayland_decoration_handle_error_csd(struct libdecor *decorator, enum libdecor_error error, const char *message) {
    LOG_ERROR("libdecor error: %s", message);
}

void wayland_decoration_handle_configure_csd(struct libdecor_frame *frame, struct libdecor_configuration *configuration, void *data) {
    window *window = data;
    i32 width = 0, height = 0;
    struct libdecor_state *state;

    if (!libdecor_configuration_get_window_state(configuration, &window->platform_state->state)) {
        window->platform_state->state = LIBDECOR_WINDOW_STATE_NONE;
    }

    libdecor_configuration_get_content_size(configuration, frame, &width, &height);

    window->width = (width == 0) ? window->platform_state->floating_width : width;
    window->height = (height == 0) ? window->platform_state->floating_height : height;

    state = libdecor_state_new(window->width, window->height);
    libdecor_frame_commit(frame, state, configuration);
    libdecor_state_free(state);

    if (libdecor_frame_is_floating(frame)) {
        window->platform_state->floating_width = window->width;
        window->platform_state->floating_height = window->height;
    }

    if (window->platform_state->first_resize) {
        window->platform_state->first_resize = FALSE;
    } else {
        window->resizing = TRUE;
        window->frames_since_resize = 0;

        event_data event = { .vec2f = { .x = window->width, .y = window->height } };
        event_fire(EVENT_TYPE_WINDOW_RESIZED, event);
    }
}

void wayland_decoration_handle_close_csd(struct libdecor_frame *frame, void *data) {
    linux_adapter *adapter = ((window *)data)->platform_state->adapter;

    if (adapter->platform_state->window_closed_callback != NULL) {
        adapter->platform_state->window_closed_callback(data);
    }
}

void wayland_decoration_handle_commit_csd(struct libdecor_frame *frame, void *data) {
    wl_surface_commit(((window *)data)->platform_state->surface);
}

void wayland_decoration_handle_dismiss_popup_csd(struct libdecor_frame *frame, const char *seat_name, void *data) {
    (void)frame;
    (void)seat_name;
    (void)data;
}

b8 wayland_setup_csd(window *window) {
    linux_adapter *adapter = window->platform_state->adapter;

    window->platform_state->frame = libdecor_decorate(adapter->adapter_state->decorator, window->platform_state->surface, &adapter->adapter_state->libdecor_frame_iface, window);

    libdecor_frame_set_title(window->platform_state->frame, window->title);
    libdecor_frame_map(window->platform_state->frame);

    return TRUE;
}
