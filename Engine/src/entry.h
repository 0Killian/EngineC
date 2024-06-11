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

#include "common.h"
#include "application.h"

// TODO: Remove these
#include "platform/platform.h"
#include <stdlib.h>

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
extern b8 create_application(application* app);

/**
 * @brief The entry point of the application.
 *
 * @retval 0 Success
 * @retval 1 Application creation error
 * @retval 2 Application hooks not defined
 * @retval 3 Engine initialization error
 * @retval 4 Application initialization error
 */
int main(void) {
    // Create the application
    application app = {};
    if (!create_application(&app)) {
        // TODO: Log
        return 1;
    }

    // Ensure that all necessary fields are set
    if (!app.init || !app.deinit) {
        // TODO: Log
        return 2;
    }

    // Initialization
    // TODO: Initialize the engine here
    u64 size_requirement;
    if (!platform_init(NULL, &size_requirement)) {
        // TODO: Log
        return 3;
    }
    void* state = malloc(size_requirement);
    if (!platform_init(state, &size_requirement)) {
        // TODO: Log
        return 3;
    }

    if (!app.init(&app)) {
        // TODO: Log
        return 4;
    }

    // Start the main loop
    // TODO: Start the main loop here

    // Deinitialization
    // TODO: Deinitialize the engine here
    app.deinit(&app);
    platform_deinit(state);
    free(state);

    return 0;
}