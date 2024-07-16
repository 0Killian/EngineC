#include "engine.h"
#include "application.h"
#include "platform/platform.h"
#include "core/log.h"
#include "core/memory.h"

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
    // Initialize the memory management system
    if (!mem_init()) {
        LOG_ERROR("Failed to initialize the memory management system");
        return FALSE;
    }

    // Allocating engine state
    app->engine_state = mem_alloc(MEMORY_TAG_UNKNOWN, sizeof(engine_state));
    mem_zero(app->engine_state, sizeof(engine_state));
    engine_state* state = (engine_state*)app->engine_state;

    // Initializing platform layer
    u64 size_requirement;
    if (!platform_init(NULL, &size_requirement)) {
        LOG_ERROR("Failed to initialize the platform layer");
        return FALSE;
    }

    state->platform_state = mem_alloc(MEMORY_TAG_UNKNOWN, size_requirement);
    if (!platform_init(state->platform_state, &size_requirement)) {
        LOG_ERROR("Failed to initialize the platform layer");
        return FALSE;
    }

    // Initializing logging system
    if (!log_init(NULL, &size_requirement)) {
        LOG_ERROR("Failed to initialize the logging system");
        return FALSE;
    }

    state->logging_state = mem_alloc(MEMORY_TAG_UNKNOWN, size_requirement);
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
    engine_state* state = (engine_state*)app->engine_state;

    // Deinitialize layers and systems
    if (state->logging_state) {
        log_deinit(state->logging_state);
    }

    if (state->platform_state) {
        platform_deinit(state->platform_state);
    }

    // Free engine state
    mem_free(state->logging_state);
    mem_free(state->platform_state);
    mem_free(state);

    // Deinitialize the memory management system, reporting any leaks in the process
    mem_deinit();
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