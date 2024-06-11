#include "platform.h"

#if PLATFORM_WINDOWS

/** @brief State of the platform layer. */
typedef struct platform_state {    
} platform_state;

static platform_state *state;

/**
 * @brief Initializes the platform layer.
 * 
 * Should be called twice, once to get the required allocation size (with state == NULL), and a second time to actually
 * initialize the platform (with state != NULL).
 * 
 * @param [in] state_storage A pointer to a memory region to store the state of the platform layer. To obtain the needed size, pass NULL.
 * @param [out] size_requirement A pointer to the size of the memory that should be allocated.
 * @retval TRUE Success
 * @retval FALSE Failure
 */
b8 platform_init(void* state_storage, u64* size_requirement) {
    if (state == NULL) {
        // Fill out the size requirement and boot out
        *size_requirement = sizeof(platform_state);
        return TRUE;
    }

    state = state_storage;

    return TRUE;
}

/**
 * @brief Deinitializes the platform layer.
 * 
 * @param [in] state A pointer to the state of the platform layer.
 */
void platform_deinit(void* state) {
    // There is nothing to do here...
}

#endif