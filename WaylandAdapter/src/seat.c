#include "seat.h"
#include "wayland_adapter.h"
#include <platform/linux_adapter.h>

void wayland_seat_handle_name(void *data, struct wl_seat *wl_seat, const char *name) {
    (void)data;
    (void)wl_seat;
    (void)name;
}

void wayland_seat_handle_capabilities(void *data, struct wl_seat *wl_seat, u32 capabilities) {
    linux_adapter *adapter = (linux_adapter *)data;

    if (adapter->adapter_state->pointer != NULL) {
        wl_pointer_destroy(adapter->adapter_state->pointer);
        adapter->adapter_state->pointer = NULL;
    }

    if (adapter->adapter_state->keyboard != NULL) {
        wl_keyboard_destroy(adapter->adapter_state->keyboard);
        adapter->adapter_state->keyboard = NULL;
    }

    if (capabilities & WL_SEAT_CAPABILITY_POINTER) {
        adapter->adapter_state->pointer = wl_seat_get_pointer(wl_seat);
        wl_pointer_add_listener(adapter->adapter_state->pointer, &adapter->adapter_state->pointer_listener, adapter);
    }

    if (capabilities & WL_SEAT_CAPABILITY_KEYBOARD) {
        adapter->adapter_state->keyboard = wl_seat_get_keyboard(wl_seat);
        wl_keyboard_add_listener(adapter->adapter_state->keyboard, &adapter->adapter_state->keyboard_listener, adapter);
    }
}
