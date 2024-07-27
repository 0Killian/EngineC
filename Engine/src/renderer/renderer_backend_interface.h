/**
 * @file plugins.h
 * @author Killian Bellouard (killianbellouard@gmail.com)
 * @brief This file defines the interface to the renderer backend implemented in a separate plugin.
 * @version 0.1
 * @date 2024-07-21
 */

#pragma once

#include "common.h"
#include "platform/platform.h"
#include "frame_packet.h"

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
     * @param[in] backend A pointer to the backend interface.
     * @param[in] config A pointer to the configuration.
     * @param[in] window A pointer to the window.
     * 
     * @retval TRUE Success
     * @retval FALSE Failure
     */
    b8 (*init)(struct renderer_backend_interface *backend, renderer_backend_config *config, const window *window);

    /**
     * @brief Deinitializes the renderer backend.
     * 
     * @param[in] backend A pointer to the backend interface.
     */
    void (*deinit)(struct renderer_backend_interface *backend);

    /**
     * @brief Prepares a frame for rendering.
     *
     * @param[in] backend A pointer to the backend interface.
     * @param[in,out] packet A pointer to the frame packet.
     *
     * @retval TRUE Success
     * @retval FALSE Failure
     */
    b8 (*frame_prepare)(struct renderer_backend_interface *backend, frame_packet *packet);

    /**
     * @brief Begins a command list.
     *
     * @param[in] backend A pointer to the backend interface.
     * @param[in,out] packet A pointer to the frame packet.
     *
     * @retval TRUE Success
     * @retval FALSE Failure
     */
    b8 (*command_list_begin)(struct renderer_backend_interface *backend, frame_packet *packet);

    /**
     * @brief Ends a command list.
     *
     * @param[in] backend A pointer to the backend interface.
     * @param[in,out] packet A pointer to the frame packet.
     *
     * @retval TRUE Success
     * @retval FALSE Failure
     */
    b8 (*command_list_end)(struct renderer_backend_interface *backend, frame_packet *packet);

    /**
     * @brief Renders the frame.
     *
     * @param[in] backend A pointer to the backend interface.
     * @param[in,out] packet A pointer to the frame packet.
     */
    b8 (*frame_render)(struct renderer_backend_interface *backend, frame_packet *packet);
} renderer_backend_interface;
