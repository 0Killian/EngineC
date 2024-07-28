#include "input.h"
#include "core/memory.h"
#include "event.h"
#include "math/math.h"

#define LOG_SCOPE "INPUT SYSTEM"
#include "core/log.h"

#define KEY_REPEAT_DELAY_MS 500
#define MOUSE_BUTTON_CLICK_DELAY_MS 250

typedef struct mouse_button_state {
    b8 down;
    b8 is_dragging;
    u32 duration_ms;
    vec2f begin;
} mouse_button_state;

typedef struct key_state {
    b8 down;
    u32 repeat_duration_ms;
} key_state;

struct input_system_state {
    b8 key_repeat_enabled;

    key_state keys[KEY_MAX_KEYS];
    mouse_button_state mouse_buttons[MAX_MOUSE_BUTTONS];
    f32 mouse_wheel_delta;
    vec2f current_mouse_pos;
    vec2f last_mouse_pos;

    uuid key_pressed_handler;
    uuid key_released_handler;
    uuid mouse_moved_handler;
    uuid mouse_button_pressed_handler;
    uuid mouse_button_released_handler;
    uuid mouse_wheel_handler;
};

static input_system_state *state;

static void event_handler(event_type type, event_data data, void *user_data);

/**
 * @brief Initializes the input system.
 *
 * Should be called twice, once to get the required allocation size (with state == NULL), and a second time to actually
 * initialize the input system (with state != NULL).
 *
 * @note This system depends on the event system, and should be initialized after it.
 *
 * @param[in] state A pointer to a memory region to store the state of the input system. To obtain the needed size, pass NULL.
 * @param[out] size_requirement A pointer to the size of the memory that should be allocated.
 * @retval TRUE Success
 * @retval FALSE Failure
 */
b8 input_init(input_system_state *state_storage, u64 *size_requirement) {
    if (state_storage == NULL) {
        *size_requirement = sizeof(input_system_state);
        return TRUE;
    }

    state = (input_system_state *)state_storage;
    mem_zero(state, sizeof(input_system_state));
    state->key_pressed_handler = INVALID_UUID;
    state->key_released_handler = INVALID_UUID;
    state->mouse_moved_handler = INVALID_UUID;
    state->mouse_button_pressed_handler = INVALID_UUID;
    state->mouse_button_released_handler = INVALID_UUID;
    state->mouse_wheel_handler = INVALID_UUID;

    b8 result = event_register_callback(EVENT_TYPE_KEY_PRESSED, event_handler, state, &state->key_pressed_handler);
    result = result && event_register_callback(EVENT_TYPE_KEY_RELEASED, event_handler, state, &state->key_released_handler);
    result = result && event_register_callback(EVENT_TYPE_MOUSE_MOVED, event_handler, state, &state->mouse_moved_handler);
    result = result &&
             event_register_callback(EVENT_TYPE_MOUSE_BUTTON_PRESSED, event_handler, state, &state->mouse_button_pressed_handler);
    result = result && event_register_callback(
                           EVENT_TYPE_MOUSE_BUTTON_RELEASED, event_handler, state, &state->mouse_button_released_handler);
    result = result && event_register_callback(EVENT_TYPE_MOUSE_WHEEL, event_handler, state, &state->mouse_wheel_handler);

    if (!result) {
        LOG_ERROR("Failed to register event callbacks");
        return FALSE;
    }

    return TRUE;
}

/**
 * @brief Deinitializes the input system.
 *
 * @param[in] state A pointer to the state of the input system.
 */
void input_deinit(input_system_state *state_storage) {
    if (state) {
        if (state->key_pressed_handler != INVALID_UUID) {
            event_unregister_callback(EVENT_TYPE_KEY_PRESSED, state->key_pressed_handler);
        }

        if (state->key_released_handler != INVALID_UUID) {
            event_unregister_callback(EVENT_TYPE_KEY_RELEASED, state->key_released_handler);
        }

        if (state->mouse_moved_handler != INVALID_UUID) {
            event_unregister_callback(EVENT_TYPE_MOUSE_MOVED, state->mouse_moved_handler);
        }

        if (state->mouse_button_pressed_handler != INVALID_UUID) {
            event_unregister_callback(EVENT_TYPE_MOUSE_BUTTON_PRESSED, state->mouse_button_pressed_handler);
        }

        if (state->mouse_button_released_handler != INVALID_UUID) {
            event_unregister_callback(EVENT_TYPE_MOUSE_BUTTON_RELEASED, state->mouse_button_released_handler);
        }

        if (state->mouse_wheel_handler != INVALID_UUID) {
            event_unregister_callback(EVENT_TYPE_MOUSE_WHEEL, state->mouse_wheel_handler);
        }

        mem_zero(state, sizeof(input_system_state));
    }

    state = NULL;
}

/**
 * @brief Update the input system.
 *
 * Should be called at the start of every frame.
 *
 * @param[in] delta_time The last frame time, in seconds.
 */
void input_update(f32 delta_time) {
    state->mouse_wheel_delta = 0.0f;
    state->last_mouse_pos = state->current_mouse_pos;

    if (state->key_repeat_enabled) {
        for (u32 i = 0; i < KEY_MAX_KEYS; i++) {
            if (state->keys[i].down) {
                state->keys[i].repeat_duration_ms += (u32)(delta_time * 1000.0f);
                if (state->keys[i].repeat_duration_ms >= KEY_REPEAT_DELAY_MS) {
                    state->keys[i].repeat_duration_ms = 0.0f;
                    event_fire(EVENT_TYPE_KEY_PRESSED, (event_data){ .key = (key)i });
                }
            }
        }
    }

    for (u32 i = 0; i < MAX_MOUSE_BUTTONS; i++) {
        state->mouse_buttons[i].duration_ms += (u32)(delta_time * 1000.0f);
    }
}

/**
 * @brief Enables of disables the key repeat mechanism.
 *
 * @param[in] enabled Whether to enable or disable the key repeat mechanism.
 */
void input_enable_key_repeat(b8 enabled) { state->key_repeat_enabled = enabled; }

/**
 * @brief Get the state of a key.
 *
 * @param[in] key The key to get the state of.
 *
 * @retval TRUE The key is pressed.
 * @retval FALSE The key is not pressed.
 */
API b8 input_is_key_down(key key) { return state->keys[key].down; }

/**
 * @brief Get the state of a mouse button.
 *
 * @param[in] button The button to get the state of.
 *
 * @retval TRUE The button is pressed.
 * @retval FALSE The button is not pressed.
 */
API b8 input_is_mouse_button_down(u32 button) { return state->mouse_buttons[button].down; }

/**
 * @brief Get the current mouse position.
 *
 * @return The current mouse position.
 */
API vec2f input_get_mouse_position() { return state->current_mouse_pos; }

/**
 * @brief Get the mouse delta for the current frame.
 *
 * @return The mouse delta for the current frame.
 */
API vec2f input_get_mouse_delta() { return vec2f_sub(state->current_mouse_pos, state->last_mouse_pos); }

/**
 * @brief Get the mouse wheel delta for the current frame.
 *
 * @return The mouse wheel delta for the current frame.
 */
API f32 input_get_mouse_wheel_delta() { return state->mouse_wheel_delta; }

/**
 * @brief Indicates if the mouse is being moved while the provided button is held down.
 *
 * @param[in] button The button to check.
 *
 * @retval TRUE The mouse is being moved while the button is held down.
 * @retval FALSE The mouse is not being moved while the button is held down.
 */
API b8 input_is_mouse_dragging(u32 button) { return state->mouse_buttons[button].is_dragging; }

static void event_handler(event_type type, event_data data, void *user_data) {
    switch (type) {
    case EVENT_TYPE_KEY_PRESSED: {
        state->keys[data.key].down = TRUE;
    } break;

    case EVENT_TYPE_KEY_RELEASED: {
        state->keys[data.key].down = FALSE;
    } break;

    case EVENT_TYPE_MOUSE_MOVED: {
        // Check for dragging
        vec2f old_pos = state->current_mouse_pos;
        state->current_mouse_pos = data.vec2f;
        for (u32 i = 0; i < MAX_MOUSE_BUTTONS; i++) {
            if (state->mouse_buttons[i].down) {
                if (state->mouse_buttons[i].is_dragging) {
                    event_data new_data = {
                        .drag = {
                            .begin = state->mouse_buttons[i].begin,
                            .button = i,
                            .current = state->current_mouse_pos,
                        }, 
                    };

                    event_fire(EVENT_TYPE_MOUSE_DRAGGED, new_data);
                } else {
                    state->mouse_buttons[i].is_dragging = TRUE;
                    state->mouse_buttons[i].begin = old_pos;

                    event_data new_data = {
                        .drag = {
                            .begin = state->mouse_buttons[i].begin,
                            .button = i,
                            .current = state->current_mouse_pos,
                        },
                    };

                    event_fire(EVENT_TYPE_MOUSE_DRAGGED, new_data);
                }
            }
        }
    } break;

    case EVENT_TYPE_MOUSE_WHEEL: {
        state->mouse_wheel_delta += data.f32;
    } break;

    case EVENT_TYPE_MOUSE_BUTTON_PRESSED: {
        state->mouse_buttons[data.u32].down = TRUE;
    } break;

    case EVENT_TYPE_MOUSE_BUTTON_RELEASED: {
        state->mouse_buttons[data.u32].down = FALSE;

        if (state->mouse_buttons[data.u32].is_dragging) {
            state->mouse_buttons[data.u32].is_dragging = FALSE;

            event_data new_data = {
                .drag = {
                    .begin = state->mouse_buttons[data.u32].begin,
                    .button = data.u32,
                    .current = state->current_mouse_pos,
                }
            };

            event_fire(EVENT_TYPE_MOUSE_DRAGGED, new_data);
        } else if (state->mouse_buttons[data.u32].duration_ms <= MOUSE_BUTTON_CLICK_DELAY_MS) {
            event_fire(EVENT_TYPE_MOUSE_BUTTON_CLICKED, (event_data){ .u32 = data.u32 });
        }
    } break;

    default: {
        LOG_ERROR("Unhandled event type: %d", type);
    } break;
    }
}
