/**
 * @file plugins.h
 * @author Killian Bellouard (killianbellouard@gmail.com)
 * @brief This file defines the engine's interface to plugins.
 * @version 0.1
 * @date 2024-07-21
 */

#pragma once

#include "common.h"
#include "platform/platform.h"

/**
 * @brief Initializes a plugin.
 * 
 * @param[out] state A pointer to store the state of the plugin.
 * 
 * @retval TRUE Success
 * @retval FALSE Failure
 */
typedef b8 (*PFN_plugin_init)(void **state);

/**
 * @brief Deinitializes a plugin.
 * 
 * @param[in] state A pointer to the interface of the plugin.
 */
typedef void (*PFN_plugin_deinit)(void *state);

/** @brief The global struct defining the interface of a plugin. */
typedef struct plugin_interface {
    /** @brief Initializes the plugin. */
    PFN_plugin_init init;
    /** @brief Deinitializes the plugin. */
    PFN_plugin_deinit deinit;

    /** @brief The state of the plugin. */
    void *state;
} plugin_interface;

/** @brief A plugin hook into the engine. */
typedef struct plugin {    
    /** @brief The name of the plugin. */
    const char *name;

    /** @brief The interface of the plugin. */
    plugin_interface interface;

    /** @brief The library handle to the plugin. */
    dynamic_library library;
} plugin;

/**
 * @brief Initializes the plugin system.
 * 
 * Should be called twice, once to get the required allocation size (with state == NULL), and a second time to actually
 * initialize the plugin system (with state != NULL).
 * 
 * @param[in] state A pointer to a memory region to store the state of the plugin system. To obtain the needed size, pass NULL.
 * @param[out] size_requirement A pointer to the size of the memory that should be allocated.
 * 
 * @retval TRUE Success
 * @retval FALSE Failure
 */
b8 plugins_init(void *state, u64 *size_requirement);

/**
 * @brief Deinitializes the plugin system.
 * 
 * @param[in] state A pointer to the state of the plugin system.
 */
void plugins_deinit(void *state);

/**
 * @brief Loads a plugin from its name.
 * 
 * @param[in] name The name of the plugin to load.
 * @param[out] result A pointer to a memory region to store the result.
 * 
 * @retval TRUE Success
 * @retval FALSE Failure
 */
b8 plugins_load(const char *name, plugin *result);

/**
 * @brief Unloads a plugin from its name.
 * 
 * The plugin is a dynamic library located in the plugins folder.
 * 
 * @warning Make sure that no copies of the plugin are still in use.
 * 
 * @param[in] name The name of the plugin to unload.
 * 
 * @retval TRUE Success
 * @retval FALSE Failure
 */
b8 plugins_unload(const char *name);

/**
 * @brief Gets a plugin from its name.
 * 
 * @param[in] name The name of the plugin to get.
 * @param[out] plugin A pointer to a memory region to store the plugin.
 * 
 * @retval TRUE Success
 * @retval FALSE Failure
 */
b8 plugins_get(const char *name, plugin *plugin);