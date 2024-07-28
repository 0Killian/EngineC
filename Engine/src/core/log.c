#include "log.h"
#include "platform/filesystem.h"
#include "platform/platform.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#undef LOG_SCOPE
#define LOG_SCOPE "LOGGING"

/** @brief The state of the logging system */
struct log_system_state {
    filesystem_handle log_file;
};

static log_system_state *state = NULL;

/**
 * @brief Logs a message at the given level.
 *
 * @param[in] level The level of the message
 * @param[in] scope The scope of the message, or NULL if the scope is global
 * @param[in] message The message to log
 * @param[in] ... The arguments to the message
 */
API void log_output(log_level level, const char *scope, const char *message, ...) {
    // TODO: Move this to another thread
    const char *level_names[] = { "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL" };
    platform_console_color level_colors_foreground[] = {
        PLATFORM_CONSOLE_COLOR_CYAN,   PLATFORM_CONSOLE_COLOR_BLUE, PLATFORM_CONSOLE_COLOR_GREEN,
        PLATFORM_CONSOLE_COLOR_YELLOW, PLATFORM_CONSOLE_COLOR_RED,  PLATFORM_CONSOLE_COLOR_RED,
    };

    platform_console_color level_colors_background[] = {
        PLATFORM_CONSOLE_COLOR_BLACK, PLATFORM_CONSOLE_COLOR_BLACK, PLATFORM_CONSOLE_COLOR_BLACK,
        PLATFORM_CONSOLE_COLOR_BLACK, PLATFORM_CONSOLE_COLOR_BLACK, PLATFORM_CONSOLE_COLOR_WHITE,
    };

    // NOTE: Imposes a 16KiB character limit, but no log should be longer than that
    char buffer[16384] = {};

    // Prefix the message
    // TODO: Use string library
    u64 offset = 0;
    if (scope) {
        offset = snprintf(buffer, sizeof(buffer), "%s: [%s] ", scope, level_names[level]);
    } else {
        offset = snprintf(buffer, sizeof(buffer), "%s: ", level_names[level]);
    }

    // Format original message
    __builtin_va_list args;
    va_start(args, message);
    u64 size = vsnprintf(buffer + offset, sizeof(buffer), message, args);
    va_end(args);

    if (offset + size + 1 > sizeof(buffer)) {
        LOG_WARN("Next message is too long to fit in the buffer. Please increase the buffer size or decrease the message size");
    }

    buffer[offset + size] = 0;

    // Print the message
    if (level >= LOG_LEVEL_ERROR) {
        platform_console_write_error(level_colors_foreground[level], level_colors_background[level], buffer);
        platform_console_write_error(level_colors_foreground[LOG_LEVEL_INFO], level_colors_background[LOG_LEVEL_INFO], "\n");
    } else {
        platform_console_write(level_colors_foreground[level], level_colors_background[level], buffer);
        platform_console_write(level_colors_foreground[LOG_LEVEL_INFO], level_colors_background[LOG_LEVEL_INFO], "\n");
    }

    buffer[offset + size] = '\n';

    if (state != NULL && state->log_file != NULL) {
        if (!filesystem_handle_write(state->log_file, buffer, offset + size + 1)) {
            b8 result = filesystem_handle_close(state->log_file);
            state->log_file = NULL;

            if (!result) {
                LOG_WARN("Failed to close log file");
            }

            LOG_WARN("Failed to write to log file, logging to console only");
        }
    }
}

/**
 * @brief Initializes the logging system.
 *
 * Should be called twice, once to get the required allocation size (with state == NULL), and a second time to actually
 * initialize the logging system (with state != NULL).
 *
 * Before a call to this function, the logging facilities will work with missing optional features (log to file, etc.).
 *
 * @param[in] state_storage A pointer to a memory region to store the state of the logging system. To obtain the needed size,
 * pass NULL.
 * @param[out] size_requirement A pointer to the size of the memory that should be allocated.
 * @retval TRUE Success
 * @retval FALSE Failure
 */
API b8 log_init(log_system_state *state_storage, u64 *size_requirement) {
    if (state_storage == NULL) {
        *size_requirement = sizeof(log_system_state);
        return TRUE;
    }

    state = (log_system_state *)state_storage;

    if (!filesystem_handle_open("log.txt", FILESYSTEM_OPEN_MODE_WRITE, &state->log_file)) {
        state->log_file = NULL;
        LOG_WARN("Failed to open log file");
    }

    return TRUE;
}

/**
 * @brief Deinitializes the logging system.
 *
 * @param[in] state A pointer to the state of the logging system.
 */
API void log_deinit(log_system_state *state) {
    if (state != NULL && state->log_file != NULL) {
        b8 result = filesystem_handle_close(state->log_file);
        state->log_file = NULL;
        if (!result) {
            LOG_WARN("Failed to close log file");
        }

        state = NULL;
    }
}
