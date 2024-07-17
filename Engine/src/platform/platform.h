/**
 * @file platform.h
 * @author Killian Bellouard (killianbellouard@gmail.com)
 * @brief This file defines the interface of the platform layer. Each platform must implement this interface in a separate .c
 * file.
 * @version 0.4
 * @date 2024-06-11
 */

#pragma once

#include "common.h"

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
    PLATFORM_CONSOLE_COLOR_BRIGHT_WHITE
} platform_console_color;

typedef void* dynamic_library;

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
b8 platform_init(void* state, u64* size_requirement);

/**
 * @brief Deinitializes the platform layer.
 * 
 * @param[in] state A pointer to the state of the platform layer.
 */
void platform_deinit(void* state);

/**
 * @brief Writes a message to the console.
 * 
 * Message considered as errors should use @ref platform_console_write_error instead.
 * 
 * @param[in] foreground The color of the text.
 * @param[in] background The color of the background.
 * @param[in] message The message to write.
 */
void platform_console_write(platform_console_color foreground, platform_console_color background, const char* message);

/**
 * @brief Writes an error message to the console.
 * 
 * @param[in] foreground The color of the text.
 * @param[in] background The color of the background.
 * @param[in] message The message to write.
 */
void platform_console_write_error(platform_console_color foreground, platform_console_color background, const char* message);

/**
 * @brief Allocates a region of memory.
 * 
 * @param[in] size The size of the region to allocate.
 * @retval A pointer to the allocated region.
 */
void* platform_allocate(u64 size);

/**
 * @brief Frees a region of memory.
 * 
 * @param[in] pointer A pointer to the region to free.
 */
void platform_free(void* pointer);

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
void* platform_get_caller();
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
b8 platform_dynamic_library_open(const char* name, dynamic_library* result);

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
b8 platform_dynamic_library_get_symbol(dynamic_library library, const char* name, void** result);