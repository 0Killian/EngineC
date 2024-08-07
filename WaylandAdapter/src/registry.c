#include "registry.h"
#include "wayland_adapter.h"
#include <core/str.h>
#include <platform/linux_adapter.h>

#define LOG_SCOPE "WAYLAND REGISTRY"
#include <core/log.h>

void wayland_registry_handle_global(void *data, struct wl_registry *registry, u32 name, const char *interface, u32 version) {
    linux_adapter *adapter = (linux_adapter *)data;

    if (interface == NULL) {
        LOG_ERROR("interface is NULL");
        return;
    }

    if (str_eq(interface, "wl_compositor")) {
        adapter->adapter_state->compositor = wl_registry_bind(registry, name, &wl_compositor_interface, version);
    } else if (str_eq(interface, "wl_seat")) {
        adapter->adapter_state->seat = wl_registry_bind(registry, name, &wl_seat_interface, version);
        wl_seat_add_listener(adapter->adapter_state->seat, &adapter->adapter_state->seat_listener, adapter);
    } else if (str_eq(interface, "xdg_wm_base")) {
        adapter->adapter_state->shell = wl_registry_bind(registry, name, &xdg_wm_base_interface, version);
        xdg_wm_base_add_listener(adapter->adapter_state->shell, &adapter->adapter_state->shell_listener, adapter);
    } else if (str_eq(interface, "zxdg_decoration_manager_v1")) {
        adapter->adapter_state->decoration_manager =
            wl_registry_bind(registry, name, &zxdg_decoration_manager_v1_interface, version);
    }
}

void wayland_registry_handle_global_remove(void *data, struct wl_registry *registry, u32 name) {}
