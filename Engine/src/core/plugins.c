#include "core/plugins.h"
#include "core/dynamic_array.h"
#include "core/str.h"

#define LOG_SCOPE "PLUGIN SYSTEM"
#include "core/log.h"

typedef struct plugins_system_state {
    DYNARRAY(plugin) plugins;
} plugins_system_state;

static plugins_system_state *state = NULL;

static void plugin_unload(plugin *);

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
b8 plugins_init(void *state_storage, u64 *size_requirement) {
    if (state_storage == NULL) {
        *size_requirement = sizeof(plugins_system_state);
        return TRUE;
    }

    state = (plugins_system_state*)state_storage;
    mem_zero(state, sizeof(plugins_system_state));

    return TRUE;
}

/**
 * @brief Deinitializes the plugin system.
 * 
 * @param[in] state A pointer to the state of the plugin system.
 */
void plugins_deinit(void *_) {
    for (uuid i = 0; i < state->plugins.count; i++) {
        if (state->plugins.data[i].name == NULL) {
            continue;
        }

        plugin_unload(&state->plugins.data[i]);
    }

    DYNARRAY_CLEAR(state->plugins);
}

/**
 * @brief Loads a plugin from its name.
 * 
 * @param[in] name The name of the plugin to load.
 * @param[out] result A pointer to a memory region to store the result.
 * 
 * @retval TRUE Success
 * @retval FALSE Failure
 */
b8 plugins_load(const char *name, plugin *result) {
    for (uuid i = 0; i < state->plugins.count; i++) {
        if (str_eq(state->plugins.data[i].name, name)) {
            // Already loaded
            return TRUE;
        }
    }

    uuid found_uuid = INVALID_UUID;
    for (uuid i = 0; i < state->plugins.count; i++) {
        if (state->plugins.data[i].name == NULL) {
            found_uuid = i;
            break;
        }
    }

    if (found_uuid == INVALID_UUID) {
        found_uuid = state->plugins.count;
        DYNARRAY_PUSH(state->plugins, (plugin){});
    }

    // TODO: String functions
    state->plugins.data[found_uuid].name = mem_alloc(MEMORY_TAG_STRING, strlen(name) + 1);
    mem_copy((char*)state->plugins.data[found_uuid].name, name, strlen(name) + 1);

    LOG_TRACE("Loading plugin %s", name);

    dynamic_library library;
    if (!platform_dynamic_library_open(name, &library)) {
        LOG_ERROR("Failed to load plugin %s: Could not open library", name);
        mem_free((char*)state->plugins.data[found_uuid].name);
        state->plugins.data[found_uuid].name = NULL;
        return FALSE;
    }

    state->plugins.data[found_uuid].library = library;

    plugin_interface *interface;
    if (!platform_dynamic_library_get_symbol(library, "_plugin_interface", (void**)&interface)) {
        LOG_ERROR("Failed to load plugin %s: Could not get symbol _plugin_interface", name);
        platform_dynamic_library_close(library);
        mem_free((char*)state->plugins.data[found_uuid].name);
        state->plugins.data[found_uuid].name = NULL;
        return FALSE;
    }

    state->plugins.data[found_uuid].interface = *interface;

    PFN_plugin_init init = interface->init;
    if (init) {
        LOG_TRACE("Initializing plugin %s", name);
        if (!init(&state->plugins.data[found_uuid].interface.state)) {
            LOG_ERROR("Failed to initialize plugin %s", name);
            platform_dynamic_library_close(library);
            mem_free((char*)state->plugins.data[found_uuid].name);
            state->plugins.data[found_uuid].name = NULL;
            return FALSE;
        }
    }

    *result = state->plugins.data[found_uuid];

    return TRUE;
}

/**
 * @brief Unloads a plugin from its name.
 * 
 * The plugin is a dynamic library located in the plugins folder.
 * 
 * @param[in] name The name of the plugin to unload.
 * 
 * @retval TRUE Success
 * @retval FALSE Failure
 */
b8 plugins_unload(const char *name) {
    for (uuid i = 0; i < state->plugins.count; i++) {
        if (state->plugins.data[i].name == NULL) {
            continue;
        }

        if (strcmp(state->plugins.data[i].name, name) == 0) {
            plugin_unload(&state->plugins.data[i]);
            return TRUE;
        }
    }

    return FALSE;
}

/**
 * @brief Gets a plugin from its name.
 * 
 * @param[in] name The name of the plugin to get.
 * @param[out] plugin A pointer to a memory region to store the plugin.
 * 
 * @retval TRUE Success
 * @retval FALSE Failure
 */
b8 plugins_get(const char *name, plugin *plugin) {
    for (uuid i = 0; i < state->plugins.count; i++) {
        if (state->plugins.data[i].name == NULL) {
            continue;
        }

        if (strcmp(state->plugins.data[i].name, name) == 0) {
            *plugin = state->plugins.data[i];
            return TRUE;
        }
    }

    return FALSE;
}

static void plugin_unload(plugin *plugin) {
    PFN_plugin_deinit deinit = plugin->interface.deinit;
    if (deinit) {
        LOG_TRACE("Deinitializing plugin %s", plugin->name);
        deinit(plugin->interface.state);
    }

    if (!platform_dynamic_library_close(plugin->library)) {
        LOG_ERROR("Failed to close plugin %s", plugin->name);
    }

    mem_free((char*)plugin->name);
    mem_zero(plugin, sizeof(struct plugin));
}
