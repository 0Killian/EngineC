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
#include "renderer/frame_packet.h"

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
     * @brief Updates the application.
     * 
     * @param[in] app A pointer to the application.
     * @param[in] delta_time The delta time.
     */
    b8 (*update)(struct application *app, f32 delta_time);

    /**
     * @brief Prepares the frame for rendering.
     * 
     * @param[in] app A pointer to the application.
     * @param[in] frame_packet A pointer to the frame packet.
     */
    b8 (*prepare_frame)(struct application *app, frame_packet *frame_packet);

    /**
     * @brief Renders the frame.
     * 
     * @param[in] app A pointer to the application.
     * @param[in] frame_packet A pointer to the frame packet.
     */
    b8 (*render_frame)(struct application *app, frame_packet *frame_packet);

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
