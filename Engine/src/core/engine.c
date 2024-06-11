#include "engine.h"
#include "application.h"
#include "platform/platform.h"
#include "core/log.h"

// TODO: Remove these
#include <stdlib.h>
#include <string.h>

typedef struct engine_state {
    /** @brief The state of the platform layer. */
    void* platform_state;
    /** @brief The state of the logging system. */
    void* logging_state;
} engine_state;

/**
 * @brief Initializes the engine, its layers and systems.
 * 
 * @param[in] app A pointer to the application.
 * 
 * @retval TRUE Success
 * @retval FALSE Failure
 */
b8 engine_init(application* app) {
    // TODO: Allocate using memory management layer
    // Allocating engine state
    app->engine_state = malloc(sizeof(engine_state));
    memset(app->engine_state, 0, sizeof(engine_state));
    engine_state* state = (engine_state*)app->engine_state;

    // Initializing platform layer
    u64 size_requirement;
    if (!platform_init(NULL, &size_requirement)) {
        LOG_ERROR("Failed to initialize the platform layer");
        return FALSE;
    }

    state->platform_state = malloc(size_requirement);
    if (!platform_init(state->platform_state, &size_requirement)) {
        LOG_ERROR("Failed to initialize the platform layer");
        return FALSE;
    }

    // Initializing logging system
    if (!log_init(NULL, &size_requirement)) {
        LOG_ERROR("Failed to initialize the logging system");
        return FALSE;
    }

    state->logging_state = malloc(size_requirement);
    if (!log_init(state->logging_state, &size_requirement)) {
        LOG_ERROR("Failed to initialize the logging system");
        return FALSE;
    }

    return TRUE;
}

/**
 * @brief Deinitializes the engine, its layers and systems.
 * 
 * @param[in] app A pointer to the application.
 */
void engine_deinit(struct application* app) {
    // TODO: Deallocate using memory management layer
    engine_state* state = (engine_state*)app->engine_state;

    // Deinitializing layers and systems
    if (state->logging_state) {
        log_deinit(state->logging_state);
    }

    if (state->platform_state) {
        platform_deinit(state->platform_state);
    }

    // Deallocating engine state
    free(state->logging_state);
    free(state->platform_state);
    free(state);
}

/**
 * @brief Runs the main loop of the engine.
 * 
 * @param[in] app A pointer to the application.
 * 
 * @retval TRUE Success
 * @retval FALSE Failure
 */
b8 engine_run(struct application* app) {
    // TODO: Main loop
    return TRUE;
}