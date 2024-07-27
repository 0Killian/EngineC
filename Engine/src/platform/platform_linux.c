#ifdef PLATFORM_LINUX
#include "core/dynamic_array.h"
#include "core/log.h"
#include "core/memory.h"
#include "linux_adapter.h"
#include "platform.h"
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

static u8 convert_platform_color(platform_console_color foreground, platform_console_color background) {
    // TODO:
    return 0;
}

linux_adapter *adapter = NULL;

/**
 * @brief Initializes the platform layer.
 *
 * Should be called twice, once to get the required allocation size (with state == NULL), and a second time to actually
 * initialize the platform (with state != NULL).
 *
 * @param [in] state_storage A pointer to a memory region to store the state of the platform layer. To obtain the needed size,
 * pass NULL.
 * @param [out] size_requirement A pointer to the size of the memory that should be allocated.
 * @retval TRUE Success
 * @retval FALSE Failure
 */
b8 platform_init(void *state_storage, u64 *size_requirement) {
    if (state_storage == NULL) {
        // Fill out the size requirement and boot out
        *size_requirement = sizeof(platform_state);
        return TRUE;
    }

    platform_state *state = state_storage;
    mem_zero(state, sizeof(platform_state));

    // Detect the used display manager
    // We first check the XDG_SESSION_TYPE environment variable
    // If it is not set or invalid, we check the WAYLAND_DISPLAY environment variable
    // If it is not set or invalid, we assume it is X11
    b8 wayland = FALSE;
    b8 valid_session_type = FALSE;
    const char *xdg_session_type = getenv("XDG_SESSION_TYPE");
    if (xdg_session_type != NULL) {
        // TODO: String functions
        if (strcmp(xdg_session_type, "wayland") == 0) {
            wayland = TRUE;
            valid_session_type = TRUE;
        } else if (strcmp(xdg_session_type, "x11") == 0) {
            wayland = FALSE;
            valid_session_type = TRUE;
        }
    }

    if (!valid_session_type) {
        // TODO: String functions
        const char *wayland_display = getenv("WAYLAND_DISPLAY");
        if (wayland_display != NULL) {
            wayland = TRUE;
        }
    }

    if (wayland) {
        LOG_TRACE("Using Wayland.");
        if (!platform_dynamic_library_open("WaylandAdapter", &state->adapter_lib)) {
            LOG_ERROR("Failed to open Wayland library.");
            return FALSE;
        }
        LOG_TRACE("Wayland library opened.");
    } else {
        LOG_FATAL("X11 not supported yet.");
        return FALSE;
    }

    if (!platform_dynamic_library_get_symbol(state->adapter_lib, "_adapter", (void **)&adapter)) {
        LOG_ERROR("Failed to load symbol linux_adapter_create.");
        platform_dynamic_library_close(state->adapter_lib);
        return FALSE;
    }

    u64 adapter_state_size;
    if (!adapter->get_state_size(&adapter_state_size)) {
        LOG_ERROR("Failed to get state size.");
        platform_dynamic_library_close(state->adapter_lib);
        return FALSE;
    }

    adapter->platform_state = state;
    adapter->adapter_state = mem_alloc(MEMORY_TAG_PLATFORM, adapter_state_size);
    adapter->init(adapter);

    // DPI Awareness
    // TODO:

    return TRUE;
}

/**
 * @brief Deinitializes the platform layer.
 *
 * @param [in] state A pointer to the state of the platform layer.
 */
void platform_deinit(void *_) {
    if (adapter) {
        for (u32 i = 0; i < adapter->platform_state->windows.count; i++) {
            if (adapter->platform_state->windows.data[i]) {
                platform_window_destroy(adapter->platform_state->windows.data[i]);
            }
        }

        DYNARRAY_CLEAR(adapter->platform_state->windows);

        adapter->deinit(adapter);

        mem_free(adapter->adapter_state);
        platform_dynamic_library_close(adapter->platform_state->adapter_lib);
    }
}

/**
 * @brief Writes a message to the console.
 *
 * Message considered as errors should use @ref platform_console_write_error instead.
 *
 * @param[in] foreground The color of the text.
 * @param[in] background The color of the background.
 * @param[in] message The message to write.
 */
void platform_console_write(platform_console_color foreground, platform_console_color background, const char *message) {
    // TODO:
    printf("%s", message);
}

/**
 * @brief Writes an error message to the console.
 *
 * @param[in] foreground The color of the text.
 * @param[in] background The color of the background.
 * @param[in] message The message to write.
 */
void platform_console_write_error(platform_console_color foreground, platform_console_color background, const char *message) {
    // TODO:
    fprintf(stderr, "%s", message);
}

/**
 * @brief Allocates a region of memory.
 *
 * @param[in] size The size of the region to allocate.
 * @retval A pointer to the allocated region.
 */
void *platform_allocate(u64 size) {
    // TODO:
    return malloc(size);
}

/**
 * @brief Frees a region of memory.
 *
 * @param[in] pointer A pointer to the region to free.
 */
void platform_free(void *pointer) {
    // TODO:
    free(pointer);
}

#ifdef DEBUG
/**
 * @brief Get the address of the caller of the current function.
 *
 * @note The "current" function is the caller of this function, so the address retrieved is the 2nd address of the call stack.
 *
 * @return The address of the caller of the current function.
 */
void *platform_get_caller() { return __builtin_return_address(1); }
#endif

/**
 * @brief Opens a Dynamic Library file.
 *
 * @param [in] name The name of the library to open.
 * @param [out] result A pointer to a memory region to store the result.
 *
 * @retval TRUE Success
 * @retval FALSE Failure
 */
b8 platform_dynamic_library_open(const char *name, dynamic_library *result) {
    char *new_string = mem_alloc(MEMORY_TAG_STRING, strlen(name) + 7);
    strcpy(new_string, "lib");
    strcat(new_string, name);
    strcat(new_string, ".so");
    *result = dlopen(new_string, RTLD_LAZY | RTLD_LOCAL);

    if (*result == NULL) {
        // Try to open in the folder containing the main executable
        char executable_path[4096];
        u64 size = readlink("/proc/self/exe", executable_path, 4096);
        if (size == -1) {
            LOG_ERROR("platform_dynamic_library_open: Failed to read the link of /proc/self/exe"
                      "(could not determine executable path)");
            mem_free(new_string);
            return FALSE;
        }

        while (executable_path[size - 1] != '/') {
            executable_path[size - 1] = 0;
            size--;
        }

        char *lib_path = mem_alloc(MEMORY_TAG_STRING, strlen(executable_path) + strlen(new_string) + 1);
        strcpy(lib_path, executable_path);
        strcat(lib_path, new_string);

        *result = dlopen(lib_path, RTLD_LAZY | RTLD_LOCAL);
        mem_free(lib_path);
    }

    mem_free(new_string);
    return *result != NULL;
}

/**
 * @brief Closes a Dynamic Library file.
 *
 * @param [in] library The library to close.
 *
 * @retval TRUE Success
 * @retval FALSE Failure
 */
b8 platform_dynamic_library_close(dynamic_library library) { return dlclose(library) == 0; }

/**
 * @brief Gets a symbol from a Dynamic Library file.
 *
 * @param [in] library The library to get the symbol from.
 * @param [in] name The name of the symbol to get.
 * @param [out] result A pointer to a memory region to store the result.
 *
 * @retval TRUE Success
 * @retval FALSE Failure
 */
b8 platform_dynamic_library_get_symbol(dynamic_library library, const char *name, void **result) {
    *result = dlsym(library, name);
    return *result != NULL;
}

/**
 * @brief Creates a new window from the specified config.
 *
 * @note The window is shown immediately.
 *
 * @param [in] config The config of the window to create.
 * @param [out] result A pointer to a memory region to store the result pointer.
 *
 * @retval TRUE Success
 * @retval FALSE Failure
 */
b8 platform_window_create(const window_config *config, window **result) {
    if (result == NULL) {
        return FALSE;
    }

    window *window = mem_alloc(MEMORY_TAG_PLATFORM, sizeof(struct window));
    u32 index = 0xFFFFFFFF;
    for (u32 i = 0; i < adapter->platform_state->windows.count; i++) {
        if (adapter->platform_state->windows.data[i] == NULL) {
            adapter->platform_state->windows.data[i] = window;
            index = i;
            break;
        }
    }

    if (index == 0xFFFFFFFF) {
        DYNARRAY_PUSH(adapter->platform_state->windows, window);
        index = adapter->platform_state->windows.count - 1;
    }

    window->width = config->width;
    window->height = config->height;

    if (config->title) {
        // TODO: String functions
        window->title = (char *)mem_alloc(MEMORY_TAG_STRING, strlen(config->title) + 1);
        strcpy(window->title, config->title);
    } else {
        // TODO: String functions
        const char *title = "Engine Window";
        window->title = (char *)mem_alloc(MEMORY_TAG_STRING, strlen(title) + 1);
        strcpy(window->title, title);
    }

    adapter->window_create(adapter, config, window);

    *result = window;
    return TRUE;
}

/**
 * @brief Destroys the given window.
 *
 * @param [in] window The window to destroy.
 */
void platform_window_destroy(window *window) {
    if (window == NULL) {
        return;
    }

    adapter->window_destroy(adapter, window);

    if (window->title != NULL) {
        mem_free(window->title);
    }
    mem_free(window);

    b8 found = FALSE;
    for (uuid i = 0; i < adapter->platform_state->windows.count; i++) {
        if (adapter->platform_state->windows.data[i] == window) {
            adapter->platform_state->windows.data[i] = NULL;
            found = TRUE;
            break;
        }
    }

    if (!found) {
        LOG_WARN("Tried to destroy an unregistered window");
    }
}

/**
 * @brief Sets the window title.
 *
 * @note The title is copied, so it is safe to use a temporary string.
 *
 * @param [in] window The window to set the title of.
 * @param [in] title The title to set.
 *
 * @retval TRUE Success
 * @retval FALSE Failure
 */
b8 platform_window_set_title(window *window, const char *title) {
    LOG_ERROR("platform_window_set_title not implemented");
    return FALSE;
    if (title == NULL) {
        return FALSE;
    }

    mem_free(window->title);

    window->title = (char *)mem_alloc(MEMORY_TAG_STRING, strlen(title) + 1);
    strcpy(window->title, title);

    adapter->window_set_title(adapter, window, title);

    return TRUE;
}

/**
 * @brief Retrieves platform-specific messages and process them.
 *
 * @retval TRUE Success
 * @retval FALSE Failure
 */
b8 platform_process_messages() { return adapter->process_messages(adapter); }

/**
 * @brief Registers the callback to be called when the window is closed.
 *
 * @note There is only one callback registered at a time.
 *
 * @param [in] callback The callback to register.
 */
void platform_register_window_closed_callback(platform_window_closed_callback callback) {
    adapter->platform_state->window_closed_callback = callback;
}

/**
 * @brief Gets the time in seconds.
 *
 * @return The time in seconds.
 */
f32 platform_get_time() {
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC_RAW, &now);
    return now.tv_sec + now.tv_nsec * 0.000000001;
}

/**
 * @brief Sleeps for the specified number of milliseconds.
 *
 * @param [in] milliseconds The number of milliseconds to sleep.
 */
API void platform_sleep(u32 milliseconds) {
    struct timespec ts;
    ts.tv_sec = milliseconds / 1000;
    ts.tv_nsec = (milliseconds % 1000) * 1000000;
    nanosleep(&ts, NULL);
    return;
}

#endif
