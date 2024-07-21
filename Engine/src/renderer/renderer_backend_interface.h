/**
 * @file plugins.h
 * @author Killian Bellouard (killianbellouard@gmail.com)
 * @brief This file defines the interface to the renderer backend implemented in a separate plugin.
 * @version 0.1
 * @date 2024-07-21
 */

#pragma once

#include "common.h"

/** @brief The configuration of the renderer backend. */
typedef struct renderer_backend_config {
    /** @brief The name of the application. */
    const char *application_name;
} renderer_backend_config;

/** @brief The interface of the renderer backend. */
typedef struct renderer_backend_interface {
    /** @brief Internal data managed by the backend. */
    void *internal_data;
    
    /**
     * @brief Initializes the renderer backend, using the configuration passed in.
     * 
     * @param[in,out] backend A pointer to the backend interface.
     * @param[in] config A pointer to the configuration.
     * 
     * @retval TRUE Success
     * @retval FALSE Failure
     */
    b8 (*init)(struct renderer_backend_interface *backend, renderer_backend_config *config);

    /**
     * @brief Deinitializes the renderer backend.
     * 
     * @param[in,out] backend A pointer to the backend interface.
     */
    void (*deinit)(struct renderer_backend_interface *backend);
} renderer_backend_interface;