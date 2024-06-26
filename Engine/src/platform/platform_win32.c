#include "platform.h"

#if PLATFORM_WINDOWS

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

/** @brief State of the platform layer. */
typedef struct platform_state {    
} platform_state;

static platform_state *state;

static u8 convert_platform_color(platform_console_color foreground, platform_console_color background) {
    return foreground & 0xF | (background & 0xF) << 4;
}

/**
 * @brief Initializes the platform layer.
 * 
 * Should be called twice, once to get the required allocation size (with state == NULL), and a second time to actually
 * initialize the platform (with state != NULL).
 * 
 * @param [in] state_storage A pointer to a memory region to store the state of the platform layer. To obtain the needed size, pass NULL.
 * @param [out] size_requirement A pointer to the size of the memory that should be allocated.
 * @retval TRUE Success
 * @retval FALSE Failure
 */
b8 platform_init(void* state_storage, u64* size_requirement) {
    if (state == NULL) {
        // Fill out the size requirement and boot out
        *size_requirement = sizeof(platform_state);
        return TRUE;
    }

    state = state_storage;

    return TRUE;
}

/**
 * @brief Deinitializes the platform layer.
 * 
 * @param [in] state A pointer to the state of the platform layer.
 */
void platform_deinit(void* state) {
    // There is nothing to do here...
}

static void console_write(u8 color, const char* message, HANDLE console_handle) {
    CONSOLE_SCREEN_BUFFER_INFO buffer_info;
    GetConsoleScreenBufferInfo(console_handle, &buffer_info);

    SetConsoleTextAttribute(console_handle, color);
    OutputDebugStringA(message);
    DWORD written = 0;
    WriteConsoleA(console_handle, message, (DWORD)strlen(message), &written, NULL);

    SetConsoleTextAttribute(console_handle, buffer_info.wAttributes);
}

/**
 * @brief Writes a message to the console.
 * 
 * Message considered as errors should use @ref platform_console_write_error instead
 * 
 * @param[in] foreground The color of the text
 * @param[in] background The color of the background
 * @param[in] message The message to write
 */
void platform_console_write(platform_console_color foreground, platform_console_color background, const char* message) {
    console_write(convert_platform_color(foreground, background), message, GetStdHandle(STD_OUTPUT_HANDLE));
}

/**
 * @brief Writes an error message to the console.
 * 
 * @param[in] foreground The color of the text
 * @param[in] background The color of the background
 * @param[in] message The message to write
 */
void platform_console_write_error(platform_console_color foreground, platform_console_color background, const char* message) {
    console_write(convert_platform_color(foreground, background), message, GetStdHandle(STD_ERROR_HANDLE));
}

#endif