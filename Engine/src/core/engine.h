/**
 * @file engine.h
 * @author Killian Bellouard (killianbellouard@gmail.com)
 * @brief This file defines the engine itself. It is responsible for managing the different layers and systems.
 * @version 0.1
 * @date 2024-06-11
 */

#pragma once

#include "common.h"

// These must be predeclared because engine.h is already included in application.h
struct application;

/**
 * @brief Initializes the critical components of the engine, like the memory system.
 * 
 * @note This function must be called first to make sure that the application can use
 * early systems in @ref create_application.
 * 
 * @retval TRUE Success
 * @retval FALSE Failure
 */
API b8 engine_early_init();

/**
 * @brief Initializes the engine, its layers and systems.
 * 
 * @param[in] app A pointer to the application.
 * 
 * @retval TRUE Success
 * @retval FALSE Failure
 */
API b8 engine_init(struct application* app);

/**
 * @brief Deinitializes the engine, its layers and systems.
 * 
 * @param[in] app A pointer to the application.
 */
API void engine_deinit(struct application* app);

/**
 * @brief Runs the main loop of the engine.
 * 
 * @param[in] app A pointer to the application.
 * 
 * @retval TRUE Success
 * @retval FALSE Failure
 */
API b8 engine_run(struct application* app);
