/**
 * @file entry.h
 * @author Killian Bellouard (killianbellouard@gmail.com)
 * @brief This file defines the entry point of an application using this engine. It must be used in the main file of the
 * application. The main function must not be defined, as this header will define it. The user must implement the @ref
 * create_application function, which will be called by the entry point to configure the engine in the init phase.
 * @warning This header must be included one time only!
 * @version 0.1
 * @date 2024-06-11
 */

#pragma once

#include "application.h"
#include "common.h"
#include "core/engine.h"
#include "core/log.h"

/**
 * @brief User-provided function for creating the application.
 *
 * This function must be used to set the configure the engine, and hook the application into it.
 *
 * @param[out] app A pointer to hold the application created by this function.
 *
 * @retval TRUE Success
 * @retval FALSE Failure
 *
 * @see application
 */
extern b8 create_application(application *app);

/**
 * @brief The entry point of the application.
 *
 * @retval 0 Success
 * @retval 1 Early initalization error
 * @retval 2 Application creation error
 * @retval 3 Application hooks not defined
 * @retval 4 Engine initialization error
 * @retval 5 Application initialization error
 * @retval 6 Engine run error
 */
int main(void) {
    // Perform early initalization routines of the engine
    if (!engine_early_init()) {
        LOG_ERROR("Failed to initialize engine");
        return 1;
    }

    // Create the application
    application app = {};
    if (!create_application(&app)) {
        LOG_ERROR("Failed to create application");
        return 2;
    }

    // Ensure that all necessary fields are set
    if (!app.init || !app.deinit || !app.update || !app.prepare_frame || !app.render_frame) {
        LOG_ERROR("Application hooks not defined");
        return 3;
    }

    // Initialization
    if (!engine_init(&app)) {
        LOG_ERROR("Failed to initialize engine");
        return 4;
    }

    if (!app.init(&app)) {
        LOG_ERROR("Failed to initialize application");
        return 5;
    }

    // Start the main loop
    if (!engine_run(&app)) {
        LOG_ERROR("Failed to run engine");
        return 6;
    }

    // Deinitialization
    app.deinit(&app);
    engine_deinit(&app);

    return 0;
}
