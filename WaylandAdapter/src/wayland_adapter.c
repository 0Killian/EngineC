#include <platform/linux_adapter.h>
#include <platform/platform.h>
#include "wayland_adapter.h"
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_wayland.h>

#define LOG_SCOPE "WAYLAND ADAPTER"
#include <core/log.h>

#include "registry.h"
#include "seat.h"
#include "pointer.h"
#include "keyboard.h"
#include "surface.h"
#include "toplevel.h"
#include "decoration.h"

static void wayland_shell_handle_ping(void *data, struct xdg_wm_base *shell, u32 serial) {
    xdg_wm_base_pong(shell, serial);
}

b8 wayland_get_state_size(u64 *state_size) {
    *state_size = sizeof(struct linux_adapter_state);
    return TRUE;
}

b8 wayland_init(struct linux_adapter *adapter) {
    struct linux_adapter_state *state = adapter->adapter_state;
    mem_zero(state, sizeof(struct linux_adapter_state));

    state->registry_listener.global = wayland_registry_handle_global;
    state->registry_listener.global_remove = wayland_registry_handle_global_remove;

    state->seat_listener.name = wayland_seat_handle_name;
    state->seat_listener.capabilities = wayland_seat_handle_capabilities;

    state->shell_listener.ping = wayland_shell_handle_ping;

    state->pointer_listener.enter = wayland_pointer_handle_enter;
    state->pointer_listener.leave = wayland_pointer_handle_leave;
    state->pointer_listener.motion = wayland_pointer_handle_motion;
    state->pointer_listener.button = wayland_pointer_handle_button;
    state->pointer_listener.axis = wayland_pointer_handle_axis;
    state->pointer_listener.frame = wayland_pointer_handle_frame;
    state->pointer_listener.axis_source = wayland_pointer_handle_axis_source;
    state->pointer_listener.axis_stop = wayland_pointer_handle_axis_stop;
    state->pointer_listener.axis_discrete = wayland_pointer_handle_axis_discrete;
    state->pointer_listener.axis_value120 = wayland_pointer_handle_axis_value120;
    state->pointer_listener.axis_relative_direction = wayland_pointer_handle_axis_relative_direction;

    state->keyboard_listener.keymap = wayland_keyboard_handle_keymap;
    state->keyboard_listener.enter = wayland_keyboard_handle_enter;
    state->keyboard_listener.leave = wayland_keyboard_handle_leave;
    state->keyboard_listener.key = wayland_keyboard_handle_key;
    state->keyboard_listener.modifiers = wayland_keyboard_handle_modifiers;
    state->keyboard_listener.repeat_info = wayland_keyboard_handle_repeat_info;

    state->surface_listener.leave = wayland_surface_handle_leave;
    state->surface_listener.enter = wayland_surface_handle_enter;
    state->surface_listener.preferred_buffer_scale = wayland_surface_handle_preferred_buffer_scale;
    state->surface_listener.preferred_buffer_transform = wayland_surface_handle_preferred_buffer_transform;

    state->toplevel_listener.close = wayland_toplevel_handle_close;
    state->toplevel_listener.configure = wayland_toplevel_handle_configure;
    state->toplevel_listener.wm_capabilities = wayland_toplevel_handle_wm_capabilities;
    state->toplevel_listener.configure_bounds = wayland_toplevel_handle_configure_bounds;

    state->xdg_surface_listener.configure = wayland_xdg_surface_handle_configure;
    
    state->decoration_listener.configure = wayland_decoration_handle_configure_ssd;

    state->libdecor_iface.error = wayland_decoration_handle_error_csd;

    state->libdecor_frame_iface.configure = wayland_decoration_handle_configure_csd;
    state->libdecor_frame_iface.close = wayland_decoration_handle_close_csd;
    state->libdecor_frame_iface.commit = wayland_decoration_handle_commit_csd;

    state->pointer_focus = INVALID_UUID;
    state->keyboard_focus = INVALID_UUID;

    state->display = wl_display_connect(NULL);
    if (state->display == NULL) {
        LOG_ERROR("Failed to connect to Wayland display");
        return FALSE;
    }

    state->registry = wl_display_get_registry(state->display);
    if (state->registry == NULL) {
        LOG_ERROR("Failed to get Wayland registry");
        return FALSE;
    }

    wl_registry_add_listener(state->registry, &state->registry_listener, adapter);
    wl_display_roundtrip(state->display);

    if (state->compositor == NULL) {
        LOG_ERROR("Failed to get Wayland compositor");
        return FALSE;
    }

    if (state->seat == NULL) {
        LOG_ERROR("Failed to get Wayland seat");
        return FALSE;
    }

    if (state->shell == NULL) {
        LOG_ERROR("Failed to get Wayland shell");
        return FALSE;
    }
    
    if (state->decoration_manager == NULL) {
        LOG_WARN("Failed to get Wayland decoration manager, falling back to client side decorations");
    }

    LOG_TRACE("Initiating libdecor");
    state->decorator = libdecor_new(state->display, &state->libdecor_iface);
    LOG_TRACE("libdecor initialized");

    return TRUE;
}

b8 wayland_process_messages(struct linux_adapter *adapter) {
    if (libdecor_dispatch(adapter->adapter_state->decorator, 0) < 0) {
        return FALSE;
    }

    return TRUE;
}

void wayland_deinit(struct linux_adapter *adapter) {
    if (adapter->adapter_state->display != NULL) {
        wl_display_disconnect(adapter->adapter_state->display);
        adapter->adapter_state->display = NULL;
    }
}

b8 wayland_window_create(struct linux_adapter *adapter, const window_config *config, window *window) {
    LOG_TRACE("Creating window");
    window->platform_state = mem_alloc(MEMORY_TAG_PLATFORM, sizeof(struct window_platform_state));
    mem_zero(window->platform_state, sizeof(struct window_platform_state));

    window->platform_state->floating_width = config->width;
    window->platform_state->floating_height = config->height;
    window->platform_state->first_resize = TRUE;

    window->platform_state->adapter = adapter;

    LOG_TRACE("Creating surface");
    window->platform_state->surface = wl_compositor_create_surface(adapter->adapter_state->compositor);
    wl_surface_add_listener(window->platform_state->surface, &adapter->adapter_state->surface_listener, window);

    if (adapter->adapter_state->decoration_manager != NULL) {
        window->platform_state->xdg_surface = xdg_wm_base_get_xdg_surface(adapter->adapter_state->shell, window->platform_state->surface);
        xdg_surface_add_listener(window->platform_state->xdg_surface, &adapter->adapter_state->xdg_surface_listener, window);

        window->platform_state->toplevel = xdg_surface_get_toplevel(window->platform_state->xdg_surface);
        xdg_toplevel_add_listener(window->platform_state->toplevel, &adapter->adapter_state->toplevel_listener, window);
        xdg_toplevel_set_title(window->platform_state->toplevel, config->title);

        zxdg_decoration_manager_v1_get_toplevel_decoration(adapter->adapter_state->decoration_manager, window->platform_state->toplevel);
        zxdg_toplevel_decoration_v1_add_listener(window->platform_state->decoration, &adapter->adapter_state->decoration_listener, window);
        zxdg_toplevel_decoration_v1_set_mode(window->platform_state->decoration, ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE);
    } else {
        wayland_setup_csd(window);
    }

    wl_surface_commit(window->platform_state->surface);
    wl_display_roundtrip(adapter->adapter_state->display);
    wl_surface_commit(window->platform_state->surface);

    return TRUE;
}

b8 wayland_window_set_title(struct linux_adapter *adapter, window *window, const char *title) {
    xdg_toplevel_set_title(window->platform_state->toplevel, title);
    return TRUE;
}

void wayland_window_destroy(struct linux_adapter *adapter, window *window) {
    if (window->platform_state->decoration != NULL) {
        zxdg_toplevel_decoration_v1_destroy(window->platform_state->decoration);
        window->platform_state->decoration = NULL;
    }

    if (window->platform_state->toplevel != NULL) {
        xdg_toplevel_destroy(window->platform_state->toplevel);
        window->platform_state->toplevel = NULL;
    }

    if (window->platform_state->xdg_surface != NULL) {
        xdg_surface_destroy(window->platform_state->xdg_surface);
        window->platform_state->xdg_surface = NULL;
    }

    if (window->platform_state->surface != NULL) {
        wl_surface_destroy(window->platform_state->surface);
        window->platform_state->surface = NULL;
    }

    mem_free(window->platform_state);
    window->platform_state = NULL;
}

b8 wayland_vulkan_surface_create(VkInstance instance, VkAllocationCallbacks *allocation_callbacks, VkSurfaceKHR *surface, const window *window) {
    VkWaylandSurfaceCreateInfoKHR create_info = {
        .sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR,
        .pNext = NULL,
        .flags = 0,
        .display = window->platform_state->adapter->adapter_state->display,
        .surface = window->platform_state->surface,
    };

    VkResult result = vkCreateWaylandSurfaceKHR(instance, &create_info, allocation_callbacks, surface);
    if (result != VK_SUCCESS) {
        LOG_ERROR("Failed to create vulkan platform surface");
        return FALSE;
    }

    return TRUE;
}

typedef DYNARRAY(const char *) extension_dynarray;

void wayland_vulkan_get_required_extensions(void *extensions_raw) {
    extension_dynarray *extensions = (extension_dynarray *)extensions_raw;
    DYNARRAY_PUSH(*extensions, "VK_KHR_wayland_surface");
}

b8 wayland_vulkan_queue_supports_presentation(VkPhysicalDevice device, u32 queue_family_index) {
    return vkGetPhysicalDeviceWaylandPresentationSupportKHR(device, queue_family_index, adapter->adapter_state->display) == VK_TRUE;
}

linux_adapter _adapter = {
    .get_state_size = wayland_get_state_size,
    .init = wayland_init,
    .process_messages = wayland_process_messages,
    .deinit = wayland_deinit,
    .window_create = wayland_window_create,
    .window_set_title = wayland_window_set_title,
    .window_destroy = wayland_window_destroy,
    .vulkan_surface_create = (b8(*)(void*,void*,void**,const window*))wayland_vulkan_surface_create,
    .vulkan_get_required_extensions = wayland_vulkan_get_required_extensions,
    .vulkan_queue_supports_present = (b8(*)(void*,u32))wayland_vulkan_queue_supports_presentation
};
