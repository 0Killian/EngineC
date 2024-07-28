/**
 * @file event.h
 * @author Killian Bellouard (killianbellouard@gmail.com)
 * @brief This file defines the event system.
 * @version 0.2
 * @date 2024-07-19
 */

#pragma once

#include "common.h"
#include "math/vec2.h"
#include "input.h"

typedef struct event_system_state event_system_state;

/** @brief The different types of events. */
typedef enum event_type {
    /**
     * @brief Event fired when the application is quit.
     * The @ref event_data is unused.
     */
    EVENT_TYPE_APPLICATION_QUIT,

    /**
     * @brief Event fired when a key is pressed.
     * The @ref event_data used is @ref event_key.key (the key that was pressed)
     */
    EVENT_TYPE_KEY_PRESSED,

    /**
     * @brief Event fired after @ref EVENT_TYPE_KEY_PRESSED is fired and the pressed key is released.
     * The @ref event_data used is @ref event_data.key (the key that was released)
     */
    EVENT_TYPE_KEY_RELEASED,

    /**
     * @brief Event fired when a mouse button is pressed.
     * The @ref event_data used is @ref event_data.u32 (the button that was pressed)
     */
    EVENT_TYPE_MOUSE_BUTTON_PRESSED,

    /**
     * @brief Event fired after @ref EVENT_TYPE_MOUSE_BUTTON_PRESSED is fired and the pressed button is released.
     * The @ref event_data used is @ref event_data.u32 (the button that was released)
     */
    EVENT_TYPE_MOUSE_BUTTON_RELEASED,

    /**
     * @brief Event fired when a mouse button is clicked.
     * The @ref event_data used is @ref event_data.u32 (the button that was clicked)
     */
    EVENT_TYPE_MOUSE_BUTTON_CLICKED,

    /**
     * @brief Event fired when the mouse is moved.
     * The @ref event_data used is @ref event_data.vec2f (the mouse position)
     */
    EVENT_TYPE_MOUSE_MOVED,

    /**
     * @brief Event fired when the mouse is moved while a button is being pressed.
     * The @ref event_data used is @ref event_data.drag (the mouse position and button held down)
     */
    EVENT_TYPE_MOUSE_DRAG_BEGIN,

    /**
     * @brief Event fired after @ref EVENT_TYPE_MOUSE_DRAG_BEGIN is fired and the mouse is moved.
     * The @ref event_data used is @ref event_data.drag (the mouse position and button held down)
     */
    EVENT_TYPE_MOUSE_DRAGGED,

    /**
     * @brief Event fired after @ref EVENT_TYPE_MOUSE_DRAG_END is fired and the pressed button is released.
     * The @ref event_data used is @ref event_data.drag (the mouse position and button held down)
     */
    EVENT_TYPE_MOUSE_DRAG_END,

    /**
     * @brief Event fired when the mouse wheel is scrolled.
     * The @ref event_data used is @ref event_data.f32 (the mouse wheel delta)
     */
    EVENT_TYPE_MOUSE_WHEEL,

    /**
     * @brief Event fired when a window is resized.
     * The @ref event_data used is @ref event_data.vec2f (the window size)
     */
    EVENT_TYPE_WINDOW_RESIZED,

    /**
     * @brief Events used by the engine for debug purposes.
     * The @ref event_data used may vary.
     * */
    EVENT_TYPE_DEBUG0,
    EVENT_TYPE_DEBUG1,
    EVENT_TYPE_DEBUG2,
    EVENT_TYPE_DEBUG3,
    EVENT_TYPE_DEBUG4,
    EVENT_TYPE_DEBUG5,
    EVENT_TYPE_DEBUG6,
    EVENT_TYPE_DEBUG7,

    /**
     * @brief Custom events.
     * The @ref event_data used may vary.
     */
    EVENT_TYPE_CUSTOM_BEGIN,
    EVENT_TYPE_CUSTOM_END = 0xFE,

    /**
     * @brief The maximum number of events.
     */
    EVENT_TYPE_MAX_EVENTS
} event_type;

/**
 * @brief The data associated with an event.
 */
typedef union event_data {
    key key;
    u32 button;

    struct {
        vec2f begin;
        vec2f current;
        u32 button;
    } drag;

    vec2f vec2f;
    u32 u32;
    f32 f32;
} event_data;

/** @brief A callback for an event. */
typedef void (*event_callback)(event_type type, event_data data, void *user_data);

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
b8 event_init(event_system_state *state, u64 *size_requirement);

/**
 * @brief Deinitializes the event system.
 * 
 * @param[in] state A pointer to the state of the event system.
 */
void event_deinit(event_system_state *state);

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
API b8 event_register_callback(event_type type, event_callback callback, void *user_data, uuid *result);

/**
 * @brief Unregister a callback.
 * 
 * @param[in] type The type of the event.
 * @param[in] uuid The UUID of the callback to unregister.
 * 
 * @retval TRUE Success
 * @retval FALSE Failure
 */
API b8 event_unregister_callback(event_type type, uuid uuid);

/**
 * @brief Fires an event.
 * 
 * @param[in] type The type of the event.
 * @param[in] data The data of the event.
 * 
 * @retval TRUE Success
 * @retval FALSE Failure
 */
API b8 event_fire(event_type type, event_data data);
