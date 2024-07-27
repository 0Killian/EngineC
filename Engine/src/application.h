/**
 * @file application.h
 * @author Killian Bellouard (killianbellouard@gmail.com)
 * @brief This file defines the interface between the engine and the application using it.
 * @version 0.1
 * @date 2024-06-11
 */

#pragma once

#include "common.h"
#include "platform/platform.h"

/**
 * @brief Holds the state of the application and the engine.
 * 
 * Created by @ref create_application.
 */
typedef struct application {
    /**
     * @brief Initializes the application.
     * 
     * @param[in] app A pointer to the application.
     * 
     * @retval TRUE Success
     * @retval FALSE Failure
     */
    b8 (*init)(struct application *app);

    /**
     * @brief Deinitializes the application.
     * 
     * @param[in] app A pointer to the application.
     */
    void (*deinit)(struct application *app);

    /** @brief The state of the application. Managed by the application. */
    void *app_state;

    /** @brief The state of the engine. Managed by the engine. */
    void *engine_state;

    /** @brief The configuration of the window. */
    window_config window_config;
} application;