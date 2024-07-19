#include "engine.h"
#include "application.h"
#include "platform/platform.h"
#include "core/log.h"
#include "core/memory.h"

static void on_window_closed(const window *window);
static void on_window_resized(const window *window);

typedef struct engine_state {
    /** @brief The state of the platform layer. */
    void *platform_state;
    /** @brief The state of the logging system. */
    void *logging_state;
    /** @brief The window state. */
    window *window;
    /** @brief Whether the engine is running. */
    b8 is_running;
    /** @brief Wether the engine is suspended. */
    b8 is_suspended;
} engine_state;

engine_state *state;

/**
 * @brief Initializes the critical components of the engine, like the memory system.
 * 
 * @note This function must be called first to make sure that the application can use
 * early systems in @ref create_application.
 * 
 * @retval TRUE Success
 * @retval FALSE Failure
 */
API b8 engine_early_init() {
    // Initialize the memory management system
    if (!mem_init()) {
        LOG_ERROR("Failed to initialize the memory management system");
        return FALSE;
    }
}

/**
 * @brief Initializes the engine, its layers and systems.
 * 
 * @param[in] app A pointer to the application.
 * 
 * @retval TRUE Success
 * @retval FALSE Failure
 */
b8 engine_init(application *app) {
    // Allocating engine state
    app->engine_state = mem_alloc(MEMORY_TAG_ENGINE, sizeof(engine_state));
    mem_zero(app->engine_state, sizeof(engine_state));
    state = (engine_state *)app->engine_state;

    // Initializing platform layer
    u64 size_requirement;
    if (!platform_init(NULL, &size_requirement)) {
        LOG_ERROR("Failed to initialize the platform layer");
        return FALSE;
    }

    state->platform_state = mem_alloc(MEMORY_TAG_PLATFORM, size_requirement);
    if (!platform_init(state->platform_state, &size_requirement)) {
        LOG_ERROR("Failed to initialize the platform layer");
        return FALSE;
    }

    platform_register_window_closed_callback(on_window_closed);
    platform_register_window_resized_callback(on_window_resized);

    // Initializing logging system
    if (!log_init(NULL, &size_requirement)) {
        LOG_ERROR("Failed to initialize the logging system");
        return FALSE;
    }

    state->logging_state = mem_alloc(MEMORY_TAG_ENGINE, size_requirement);
    if (!log_init(state->logging_state, &size_requirement)) {
        LOG_ERROR("Failed to initialize the logging system");
        return FALSE;
    }

    // Create the window
    if (!platform_window_create(&app->window_config, &state->window)) {
        LOG_ERROR("Failed to create the window");
        return FALSE;
    }

    state->is_running = TRUE;

    return TRUE;
}

/**
 * @brief Deinitializes the engine, its layers and systems.
 * 
 * @param[in] app A pointer to the application.
 */
void engine_deinit(struct application *app) {
    state->is_running = FALSE;

    // Destroy the window
    if (state->window) {
        platform_window_destroy(state->window);
    }

    // Deinitialize layers and systems
    if (state->logging_state) {
        log_deinit(state->logging_state);
        mem_free(state->logging_state);
    }

    if (state->platform_state) {
        platform_deinit(state->platform_state);
        mem_free(state->platform_state);
    }

    // Free engine state
    mem_free(state);

    // Deinitialize the memory management system, reporting any leaks in the process
    mem_deinit();

    state = NULL;
}

/**
 * @brief Runs the main loop of the engine.
 * 
 * @param[in] app A pointer to the application.
 * 
 * @retval TRUE Success
 * @retval FALSE Failure
 */
b8 engine_run(struct application *app) {
    while (platform_process_messages()) {
        if (!state->is_running) {
            break;
        }

        if (state->is_suspended) {
            // TODO: Sleep for some time
            continue;
        }

        // Throttle the resizes
        if (state->window->resizing) {
            state->window->frames_since_resize++;

            if (state->window->frames_since_resize >= 30) {
                // Tell the renderer and the application that the window has resized

                state->window->resizing = FALSE;
                state->window->frames_since_resize = 0;
            } else {
                // TODO: Wait for the next frame (16ms for 60fps -> calculate based on the target FPS,
                // 0ms if unlimited)
            }

            continue;
        }

        // TODO: Implement the main loop
    }

    return TRUE;
}

static void on_window_closed(const window *window) {
    state->is_running = FALSE;
}

static void on_window_resized(const window *window) {
    if (window->width == 0 || window->height == 0) {
        LOG_INFO("Window minimized, suspending engine.");
        state->is_suspended = TRUE;
    } else {
        if (state->is_suspended) {
            LOG_INFO("Window restored, resuming engine.");
            state->is_suspended = FALSE;
        }
    }
}