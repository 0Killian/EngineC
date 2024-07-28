/**
 * @file input.h
 * @author Killian Bellouard (killianbellouard@gmail.com)
 * @brief This file defines the input system.
 * @version 0.1
 * @date 2024-07-20
 */

#pragma once

#include "common.h"
#include "math/vec2.h"

typedef struct input_system_state input_system_state;

/** @brief The different types of keys. */
typedef enum key {
    /** @brief Letters */
    KEY_A,
    KEY_B,
    KEY_C,
    KEY_D,
    KEY_E,
    KEY_F,
    KEY_G,
    KEY_H,
    KEY_I,
    KEY_J,
    KEY_K,
    KEY_L,
    KEY_M,
    KEY_N,
    KEY_O,
    KEY_P,
    KEY_Q,
    KEY_R,
    KEY_S,
    KEY_T,
    KEY_U,
    KEY_V,
    KEY_W,
    KEY_X,
    KEY_Y,
    KEY_Z,

    /** @brief Numbers */
    KEY_NUM0,
    KEY_NUM1,
    KEY_NUM2,
    KEY_NUM3,
    KEY_NUM4,
    KEY_NUM5,
    KEY_NUM6,
    KEY_NUM7,
    KEY_NUM8,
    KEY_NUM9,

    /** @brief Special keys */
    KEY_PAUSE,
    KEY_ESCAPE,
    KEY_LCONTROL,
    KEY_LSHIFT,
    KEY_LALT,
    KEY_LSYSTEM,
    KEY_RCONTROL,
    KEY_RSHIFT,
    KEY_RALT,
    KEY_RSYSTEM,
    KEY_SEMICOLON,
    KEY_COMMA,
    KEY_PERIOD,
    KEY_PIPE,
    KEY_SLASH,
    KEY_TILDE,
    KEY_EQUAL,
    KEY_DASH,
    KEY_SPACE,
    KEY_RETURN,
    KEY_BACKSPACE,
    KEY_TAB,
    KEY_PAGEUP,
    KEY_PAGEDOWN,
    KEY_END,
    KEY_HOME,
    KEY_INSERT,
    KEY_DELETE,
    KEY_LEFT,
    KEY_RIGHT,
    KEY_UP,
    KEY_DOWN,
    KEY_APOSTROPHE,
    KEY_NON_US_SLASH,
    KEY_CAPS_LOCK,
    KEY_PRINT_SCREEN,
    KEY_SCROLL_LOCK,
    KEY_LBRACE,
    KEY_RBRACE,

    /** @brief Keypad */
    KEY_KP_ADD,
    KEY_KP_SUBTRACT,
    KEY_KP_MULTIPLY,
    KEY_KP_DIVIDE,
    KEY_KP_0,
    KEY_KP_1,
    KEY_KP_2,
    KEY_KP_3,
    KEY_KP_4,
    KEY_KP_5,
    KEY_KP_6,
    KEY_KP_7,
    KEY_KP_8,
    KEY_KP_9,
    KEY_KP_DECIMAL,
    KEY_KP_ENTER,
    KEY_KP_LOCK,

    /** @brief Function keys */
    KEY_F1,
    KEY_F2,
    KEY_F3,
    KEY_F4,
    KEY_F5,
    KEY_F6,
    KEY_F7,
    KEY_F8,
    KEY_F9,
    KEY_F10,
    KEY_F11,
    KEY_F12,
    KEY_F13,
    KEY_F14,
    KEY_F15,

    /** @brief Maximum number of keys. */
    KEY_MAX_KEYS
} key;

/** @brief Maximum number of mouse buttons handled by the input system. */
#define MAX_MOUSE_BUTTONS 8

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
b8 input_init(input_system_state *input_system_state, u64 *size_requirement);

/**
 * @brief Deinitializes the input system.
 *
 * @param[in] state A pointer to the state of the input system.
 */
void input_deinit(input_system_state *state);

/**
 * @brief Update the input system.
 *
 * Should be called at the start of every frame.
 *
 * @param[in] delta_time The last frame time, in seconds.
 */
void input_update(f32 delta_time);

/**
 * @brief Enables of disables the key repeat mechanism.
 *
 * @param[in] enabled Whether to enable or disable the key repeat mechanism.
 */
void input_enable_key_repeat(b8 enabled);

/**
 * @brief Get the state of a key.
 *
 * @param[in] key The key to get the state of.
 *
 * @retval TRUE The key is pressed.
 * @retval FALSE The key is not pressed.
 */
API b8 input_is_key_down(key key);

/**
 * @brief Get the state of a mouse button.
 *
 * @param[in] button The button to get the state of.
 *
 * @retval TRUE The button is pressed.
 * @retval FALSE The button is not pressed.
 */
API b8 input_is_mouse_button_down(u32 button);

/**
 * @brief Get the current mouse position.
 *
 * @return The current mouse position.
 */
API vec2f input_get_mouse_position();

/**
 * @brief Get the mouse delta for the current frame.
 *
 * @return The mouse delta for the current frame.
 */
API vec2f input_get_mouse_delta();

/**
 * @brief Get the mouse wheel delta for the current frame.
 *
 * @return The mouse wheel delta for the current frame.
 */
API f32 input_get_mouse_wheel_delta();

/**
 * @brief Indicates if the mouse is being moved while the provided button is held down.
 *
 * @param[in] button The button to check.
 *
 * @retval TRUE The mouse is being moved while the button is held down.
 * @retval FALSE The mouse is not being moved while the button is held down.
 */
API b8 input_is_mouse_dragging(u32 button);
