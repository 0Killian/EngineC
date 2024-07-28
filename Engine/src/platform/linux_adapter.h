/**
 * @file linux_adapter.h
 * @author Killian Bellouard (killianbellouard@gmail.com)
 * @brief This file defines the interface of the linux adapter (needed because of X11 and Wayland).
 * @version 0.1
 * @date 2024-06-26
 */

#pragma once

#include "core/dynamic_array.h"
#include "platform/platform.h"

typedef struct linux_adapter_state linux_adapter_state;

/** @brief State of the platform layer. */
struct platform_system_state {
    DYNARRAY(window *) windows;
    platform_window_closed_callback window_closed_callback;
    dynamic_library adapter_lib;
};

/** @brief The interface of the linux adapter used to manage windows. */
typedef struct linux_adapter {
    b8 (*get_state_size)(u64 *state_size);
    b8 (*init)(struct linux_adapter *adapter);
    b8 (*process_messages)(struct linux_adapter *adapter);
    void (*deinit)(struct linux_adapter *adapter);

    b8 (*window_create)(struct linux_adapter *adapter, const window_config *config, window *window);
    b8 (*window_set_title)(struct linux_adapter *adapter, window *window, const char *title);
    void (*window_destroy)(struct linux_adapter *adapter, window *window);

    b8 (*vulkan_surface_create)(void *instance, void *allocation_callbacks, void **surface, const window *window);
    // DYNARRAY(const char*) *extensions
    void (*vulkan_get_required_extensions)(void *extensions);
    b8 (*vulkan_queue_supports_present)(void *device, u32 queue_family);

    struct platform_system_state *platform_state;
    struct linux_adapter_state *adapter_state;
} linux_adapter;

extern linux_adapter *adapter;
