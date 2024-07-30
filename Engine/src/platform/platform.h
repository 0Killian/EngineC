/**
 * @file platform.h
 * @author Killian Bellouard (killianbellouard@gmail.com)
 * @brief This file defines the interface of the platform layer. Each platform must implement this interface in a separate .c
 * file.
 * @version 0.5
 * @date 2024-06-11
 */

#pragma once

#include "common.h"

// Forward declarations of internal structures.
typedef struct platform_system_state platform_system_state;
typedef struct window_platform_state window_platform_state;

/** @brief The console colors used by the platform layer. */
typedef enum platform_console_color {
    PLATFORM_CONSOLE_COLOR_BLACK = 0,
    PLATFORM_CONSOLE_COLOR_BLUE,
    PLATFORM_CONSOLE_COLOR_GREEN,
    PLATFORM_CONSOLE_COLOR_CYAN,
    PLATFORM_CONSOLE_COLOR_RED,
    PLATFORM_CONSOLE_COLOR_PURPLE,
    PLATFORM_CONSOLE_COLOR_YELLOW,
    PLATFORM_CONSOLE_COLOR_WHITE,
    PLATFORM_CONSOLE_COLOR_GRAY,
    PLATFORM_CONSOLE_COLOR_LIGHT_BLUE,
    PLATFORM_CONSOLE_COLOR_LIGHT_GREEN,
    PLATFORM_CONSOLE_COLOR_LIGHT_CYAN,
    PLATFORM_CONSOLE_COLOR_LIGHT_RED,
    PLATFORM_CONSOLE_COLOR_LIGHT_PURPLE,
    PLATFORM_CONSOLE_COLOR_LIGHT_YELLOW,
    PLATFORM_CONSOLE_COLOR_BRIGHT_WHITE,
    PLATFORM_CONSOLE_COLOR_RESET,
} platform_console_color;

/** @brief A handle to a Dynamic Library. */
typedef void *dynamic_library;

/** @brief A struct describing the window to create. */
typedef struct window_config {
    i32 position_x;
    i32 position_y;
    u32 width;
    u32 height;
    const char *title;
    const char *name;
} window_config;

/** @brief The state of a window. */
typedef struct window {
    /** @brief The title of the window. */
    char *title;

    /** @brief The width of the window in pixels. */
    u32 width;
    /** @brief The height of the window in pixels. */
    u32 height;

    /** @brief The pixel density of this window (read-only). */
    f32 device_pixel_ratio;

    /** @brief Whether the window is currently being resized. */
    b8 resizing;
    /** @brief The number of frames since the last resize. */
    u16 frames_since_resize;

    /** @brief The state of the platform layer. */
    struct window_platform_state *platform_state;
} window;

// Callbacks
typedef void (*platform_window_closed_callback)(const window *window);

/**
 * @brief Initializes the platform layer.
 *
 * Should be called twice, once to get the required allocation size (with state == NULL), and a second time to actually
 * initialize the platform (with state != NULL).
 *
 * @note It is safe to call @ref platform_allocate and @ref platform_free before calling this function.
 *
 * @param[in] state A pointer to a memory region to store the state of the platform layer. To obtain the needed size, pass NULL.
 * @param[out] size_requirement A pointer to the size of the memory that should be allocated.
 * @retval TRUE Success
 * @retval FALSE Failure
 */
b8 platform_init(platform_system_state *state, u64 *size_requirement);

/**
 * @brief Deinitializes the platform layer.
 *
 * @param[in] state A pointer to the state of the platform layer.
 */
void platform_deinit(platform_system_state *state);

/**
 * @brief Writes a message to the console.
 *
 * Message considered as errors should use @ref platform_console_write_error instead.
 *
 * @param[in] foreground The color of the text.
 * @param[in] background The color of the background.
 * @param[in] message The message to write.
 */
void platform_console_write(platform_console_color foreground, platform_console_color background, const char *message);

/**
 * @brief Writes an error message to the console.
 *
 * @param[in] foreground The color of the text.
 * @param[in] background The color of the background.
 * @param[in] message The message to write.
 */
void platform_console_write_error(platform_console_color foreground, platform_console_color background, const char *message);

/**
 * @brief Allocates a region of memory.
 *
 * @param[in] size The size of the region to allocate.
 * @retval A pointer to the allocated region.
 */
void *platform_allocate(u64 size);

/**
 * @brief Frees a region of memory.
 *
 * @param[in] pointer A pointer to the region to free.
 */
void platform_free(void *pointer);

#ifdef DEBUG
/**
 * @brief Get the address of the caller of the current function.
 *
 * @note The "current" function is the caller of this function, so the address retrieved is the 2nd address of the call stack.
 *
 * @warning The MSVC compiler does not provide call stack walking intrinsics. In this case, the address will always be NULL.
 *
 * @return The address of the caller of the current function, or NULL if MSVC is used.
 */
void *platform_get_caller();
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
b8 platform_dynamic_library_open(const char *name, dynamic_library *result);

/**
 * @brief Closes a Dynamic Library file.
 *
 * @param [in] library The library to close.
 *
 * @retval TRUE Success
 * @retval FALSE Failure
 */
b8 platform_dynamic_library_close(dynamic_library library);

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
b8 platform_dynamic_library_get_symbol(dynamic_library library, const char *name, void **result);

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
b8 platform_window_create(const window_config *config, window **result);

/**
 * @brief Destroys the given window.
 *
 * @param [in] window The window to destroy.
 */
void platform_window_destroy(window *window);

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
b8 platform_window_set_title(window *window, const char *title);

/**
 * @brief Retrieves platform-specific messages and process them.
 *
 * @retval TRUE Success
 * @retval FALSE Failure
 */
b8 platform_process_messages();

/**
 * @brief Registers the callback to be called when the window is closed.
 *
 * @note There is only one callback registered at a time.
 *
 * @param [in] callback The callback to register.
 */
void platform_register_window_closed_callback(platform_window_closed_callback callback);

/**
 * @brief Gets the time in seconds.
 *
 * @return The time in seconds.
 */
f32 platform_get_time();

/**
 * @brief Sleeps for the specified number of milliseconds.
 *
 * @param [in] milliseconds The number of milliseconds to sleep.
 */
API void platform_sleep(u32 milliseconds);
