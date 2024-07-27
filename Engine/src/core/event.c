#include "event.h"
#include "core/dynamic_array.h"
#include "core/log.h"

typedef struct event_callback_entry {
    event_callback callback;
    void *user_data;
} event_callback_entry;

typedef struct event_system_state {
    DYNARRAY(event_callback_entry) callbacks[EVENT_TYPE_MAX_EVENTS];
} event_system_state;

static event_system_state *state = NULL;

/**
 * @brief Initializes the event system.
 * 
 * Should be called twice, once to get the required allocation size (with state == NULL), and a second time to actually
 * initialize the event system (with state != NULL).
 * 
 * @param[in] state A pointer to a memory region to store the state of the event system. To obtain the needed size, pass NULL.
 * @param[out] size_requirement A pointer to the size of the memory that should be allocated.
 * @retval TRUE Success
 * @retval FALSE Failure
 */
b8 event_init(void *state_storage, u64 *size_requirement) {
    if (state_storage == NULL) {
        *size_requirement = sizeof(event_system_state);
        return TRUE;
    }

    state = (event_system_state *)state_storage;
    mem_zero(state, sizeof(event_system_state));

    for (u32 i = 0; i < EVENT_TYPE_MAX_EVENTS; i++) {
        DYNARRAY_RESERVE(state->callbacks[i], 32);
    }

    return TRUE;
}

/**
 * @brief Deinitializes the event system.
 * 
 * @param[in] state A pointer to the state of the event system.
 */
void event_deinit(void *) {
    if (state != NULL) {
        for (u32 i = 0; i < EVENT_TYPE_MAX_EVENTS; i++) {
            for (u32 j = 0; j < state->callbacks[i].count; j++) {
                if (state->callbacks[i].data[j].callback) {
                    LOG_WARN("Unregistered callback left in event system: 0x%p", state->callbacks[i].data[j].callback);
                }
            }
            DYNARRAY_CLEAR(state->callbacks[i]);
        }
        mem_zero(state, sizeof(event_system_state));
    }

    state = NULL;
}

/**
 * @brief Register a callback to be called when an event is fired, identified by its type and the resulting UUID.
 * 
 * @param[in] type The type of the event.
 * @param[in] callback The callback to register.
 * @param[in] user_data The user data to pass to the callback.
 * @param[out] result A pointer to a memory region to store the resulting UUID.
 * 
 * @retval TRUE Success
 * @retval FALSE Failure
 */
b8 event_register_callback(event_type type, event_callback callback, void *user_data, uuid *result) {
    if (state == NULL || result == NULL || type >= EVENT_TYPE_MAX_EVENTS || callback == NULL) {
        return FALSE;
    }

    *result = INVALID_UUID;
    for (u32 i = 0; i < state->callbacks[type].count; i++) {
        if (state->callbacks[type].data[i].callback == NULL) {
            *result = i;
        }
    }

    if (*result == INVALID_UUID) {
        *result = state->callbacks[type].count;
        event_callback_entry entry = { .callback = callback, .user_data = user_data };
        DYNARRAY_PUSH(state->callbacks[type], entry);
    }

    return TRUE;
}

/**
 * @brief Unregister a callback.
 * 
 * @param[in] type The type of the event.
 * @param[in] uuid The UUID of the callback to unregister.
 * 
 * @retval TRUE Success
 * @retval FALSE Failure
 */
b8 event_unregister_callback(event_type type, uuid uuid) {
    if (state == NULL || type >= EVENT_TYPE_MAX_EVENTS || uuid >= state->callbacks[type].count) {
        return FALSE;
    }

    state->callbacks[type].data[uuid].callback = NULL;
    return TRUE;
}

/**
 * @brief Fires an event.
 * 
 * @param[in] type The type of the event.
 * @param[in] data The data of the event.
 * 
 * @retval TRUE Success
 * @retval FALSE Failure
 */
b8 event_fire(event_type type, event_data data) {
    if (state == NULL || type >= EVENT_TYPE_MAX_EVENTS) {
        return FALSE;
    }

    for (u32 i = 0; i < state->callbacks[type].count; i++) {
        if (state->callbacks[type].data[i].callback != NULL) {
            state->callbacks[type].data[i].callback(type, data, state->callbacks[type].data[i].user_data);
        }
    }

    return TRUE;
}
