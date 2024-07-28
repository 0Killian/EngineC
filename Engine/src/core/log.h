/**
 * @file log.h
 * @author Killian Bellouard (killianbellouard@gmail.com)
 * @brief This file defines the logging system.
 * @version 0.1
 * @date 2024-06-11
 */

#pragma once

#include "common.h"

typedef struct log_system_state log_system_state;

#ifndef LOG_SCOPE
/** @brief The scope of the log */
#define LOG_SCOPE NULL
#endif

#ifndef LOG_WARN_ENABLED
/** @brief Whether or not to log warnings */
#define LOG_WARN_ENABLED 1
#endif

#ifndef LOG_INFO_ENABLED
/** @brief Whether or not to log info */
#define LOG_INFO_ENABLED 1
#endif

#ifdef DEBUG
#ifndef LOG_DEBUG_ENABLED
/** @brief Whether or not to log debug */
#define LOG_DEBUG_ENABLED 1
#endif

#ifndef LOG_TRACE_ENABLED
/** @brief Whether or not to log trace */
#define LOG_TRACE_ENABLED 1
#endif
#else
#ifndef LOG_DEBUG_ENABLED
/** @brief Whether or not to log debug */
#define LOG_DEBUG_ENABLED 0
#endif

#ifndef LOG_TRACE_ENABLED
/** @brief Whether or not to log trace */
#define LOG_TRACE_ENABLED 0
#endif
#endif

// NOTE: This system must support its usage before the call to the init function (we must be able to log before proper
// initialization)

/** @brief Represents the levels of logging */
typedef enum log_level {
    /** @brief Trace log level, used for verbose debugging */
    LOG_LEVEL_TRACE,
    /** @brief Debug log level, used for debugging */
    LOG_LEVEL_DEBUG,
    /** @brief Info log level, used for information */
    LOG_LEVEL_INFO,
    /** @brief Warning log level, used for non-critical problems that can cause the engine to run unexpectedly */
    LOG_LEVEL_WARN,
    /** @brief Error log level, used for critical errors that will cause the engine to run unexpectedly or crash */
    LOG_LEVEL_ERROR,
    /** @brief Fatal log level, used for critical errors that will stop the application immediately */
    LOG_LEVEL_FATAL
} log_level;

/**
 * @brief Logs a message at the given level.
 *
 * @param[in] level The level of the message
 * @param[in] scope The scope of the message, or NULL if the scope is global
 * @param[in] message The message to log
 * @param[in] ... The arguments to the message
 */
API void log_output(log_level level, const char *scope, const char *message, ...);

/**
 * @brief Initializes the logging system.
 *
 * Should be called twice, once to get the required allocation size (with state == NULL), and a second time to actually
 * initialize the logging system (with state != NULL).
 *
 * Before a call to this function, the logging facilities will work with missing optional features (log to file, etc.).
 *
 * @param[in] state A pointer to a memory region to store the state of the logging system. To obtain the needed size, pass NULL.
 * @param[out] size_requirement A pointer to the size of the memory that should be allocated.
 * @retval TRUE Success
 * @retval FALSE Failure
 */
API b8 log_init(log_system_state *state, u64 *size_requirement);

/**
 * @brief Deinitializes the logging system.
 *
 * @param[in] state A pointer to the state of the logging system.
 */
API void log_deinit(log_system_state *state);

/**
 * @brief Logs a fatal error and exits the application.
 * @param[in] message The message to log
 * @param[in] ... The arguments to the message
 */
#define LOG_FATAL(message, ...) log_output(LOG_LEVEL_FATAL, LOG_SCOPE, message, ##__VA_ARGS__)

/**
 * @brief Logs an error.
 * @param[in] message The message to log
 * @param[in] ... The arguments to the message
 */
#define LOG_ERROR(message, ...) log_output(LOG_LEVEL_ERROR, LOG_SCOPE, message, ##__VA_ARGS__)

#if LOG_WARN_ENABLED == 1
/**
 * @brief Logs a warning.
 * @param[in] message The message to log
 * @param[in] ... The arguments to the message
 */
#define LOG_WARN(message, ...) log_output(LOG_LEVEL_WARN, LOG_SCOPE, message, ##__VA_ARGS__)
#else
/**
 * @brief Logs a warning.
 * @param[in] message The message to log
 * @param[in] ... The arguments to the message
 */
#define LOG_WARN(message, ...)
#endif

#if LOG_INFO_ENABLED == 1
/**
 * @brief Logs an info.
 * @param[in] message The message to log
 * @param[in] ... The arguments to the message
 */
#define LOG_INFO(message, ...) log_output(LOG_LEVEL_INFO, LOG_SCOPE, message, ##__VA_ARGS__)
#else
/**
 * @brief Logs an info.
 * @param[in] message The message to log
 * @param[in] ... The arguments to the message
 */
#define LOG_INFO(message, ...)
#endif

#if LOG_DEBUG_ENABLED == 1
/**
 * @brief Logs a debug.
 * @param[in] message The message to log
 * @param[in] ... The arguments to the message
 */
#define LOG_DEBUG(message, ...) log_output(LOG_LEVEL_DEBUG, LOG_SCOPE, message, ##__VA_ARGS__)
#else
/**
 * @brief Logs a debug.
 * @param[in] message The message to log
 * @param[in] ... The arguments to the message
 */
#define LOG_DEBUG(message, ...)
#endif

#if LOG_TRACE_ENABLED == 1
/**
 * @brief Logs a trace.
 * @param[in] message The message to log
 * @param[in] ... The arguments to the message
 */
#define LOG_TRACE(message, ...) log_output(LOG_LEVEL_TRACE, LOG_SCOPE, message, ##__VA_ARGS__)
#else
/**
 * @brief Logs a trace.
 * @param[in] message The message to log
 * @param[in] ... The arguments to the message
 */
#define LOG_TRACE(message, ...)
#endif
