#include "vulkan_command_buffer.h"
#include <core/memory.h>
#include <vulkan/vulkan_core.h>

#define LOG_SCOPE "VULKAN COMMAND BUFFER"
#include "vulkan_utils.h"

static const char *state_to_str(vulkan_command_buffer_state state) {
    switch (state) {
    case COMMAND_BUFFER_STATE_NOT_ALLOCATED: return "Not allocated";
    case COMMAND_BUFFER_STATE_READY: return "Ready";
    case COMMAND_BUFFER_STATE_RECORDING: return "Recording";
    case COMMAND_BUFFER_STATE_RENDERING: return "Rendering";
    case COMMAND_BUFFER_STATE_ENDED: return "Ended";
    case COMMAND_BUFFER_STATE_SUBMITTED: return "Submitted";
    default: return "Unknown";
    }
}

/**
 * @brief Allocates a command buffer from a pool.
 *
 * @param[in] state The vulkan state.
 * @param[in] pool The pool from which the command buffer should be allocated.
 * @param[in] name The name of the command buffer (may be used for debugging purposes).
 * @param[in] primary Indicates if the command buffer is a primary command buffer.
 * @param[out] command_buffer The command buffer.
 *
 * @retval TRUE Success
 * @retval FALSE Failure
 */
b8 vulkan_command_buffer_alloc(
    vulkan_state *state, VkCommandPool pool, const char *name, b8 primary, vulkan_command_buffer *command_buffer) {
    VkResult result;

    VkCommandBufferAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = pool,
        .level = primary ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY,
        .commandBufferCount = 1,
    };

    result = vkAllocateCommandBuffers(state->device.logical_device, &alloc_info, &command_buffer->handle);
    if (result != VK_SUCCESS) {
        LOG_ERROR("Failed to allocate command buffer %s: %s", name, vk_result_to_string(result));
        return FALSE;
    }

    VK_SET_OBJECT_DEBUG_NAME(state, VK_OBJECT_TYPE_COMMAND_BUFFER, command_buffer->handle, "CommandBuffer.", name);

    command_buffer->name = str_dup(name);
    command_buffer->state = COMMAND_BUFFER_STATE_READY;

    return TRUE;
}

/**
 * @brief Frees a command buffer.
 *
 * @param[in] state The vulkan state.
 * @param[in] pool The pool from which the command buffer was allocated.
 * @param[in] command_buffer The command buffer to free.
 */
void vulkan_command_buffer_free(vulkan_state *state, VkCommandPool pool, vulkan_command_buffer *command_buffer) {
    if (command_buffer->handle != VK_NULL_HANDLE) {
        vkFreeCommandBuffers(state->device.logical_device, pool, 1, &command_buffer->handle);
        command_buffer->handle = VK_NULL_HANDLE;
    }

    if (command_buffer->name != NULL) {
        mem_free(command_buffer->name);
        command_buffer->name = NULL;
    }

    command_buffer->state = COMMAND_BUFFER_STATE_NOT_ALLOCATED;
}

/**
 * @brief Transition the command buffer into a recording state.
 *
 * @param[in] state The vulkan state.
 * @param[in] command_buffer The command buffer.
 * @param[in] is_single_use Indicates if this command buffer will be used once.
 * @param[in] is_renderpass_continue Indicates if this command buffer will be used in a renderpass.
 * @param[in] is_simultaneous_use Indicates if this command buffer will be used in multiple command buffers.
 *
 * @retval TRUE Success
 * @retval FALSE Failure
 */
b8 vulkan_command_buffer_begin(vulkan_state *state,
                               vulkan_command_buffer *command_buffer,
                               b8 is_single_use,
                               b8 is_renderpass_continue,
                               b8 is_simultaneous_use) {
    if (command_buffer->state != COMMAND_BUFFER_STATE_READY) {
        LOG_ERROR("Command buffer %s is not in a ready state (%s)", command_buffer->name, state_to_str(command_buffer->state));
        return FALSE;
    }

    VkCommandBufferBeginInfo begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
    };

    if (is_single_use) {
        begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    }

    if (is_renderpass_continue) {
        begin_info.flags |= VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    }

    if (is_simultaneous_use) {
        begin_info.flags |= VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    }

    VkResult result = vkBeginCommandBuffer(command_buffer->handle, &begin_info);
    if (result != VK_SUCCESS) {
        LOG_ERROR("Failed to begin command buffer %s: %s", command_buffer->name, vk_result_to_string(result));
        return FALSE;
    }

    command_buffer->state = COMMAND_BUFFER_STATE_RECORDING;
    return TRUE;
}

/**
 * @brief Ends the recording of the command buffer.
 *
 * @param[in] state The vulkan state.
 * @param[in] command_buffer The command buffer.
 *
 * @retval TRUE Success
 * @retval FALSE Failure
 */
b8 vulkan_command_buffer_end(vulkan_state *state, vulkan_command_buffer *command_buffer) {
    if (command_buffer->state != COMMAND_BUFFER_STATE_RECORDING) {
        LOG_ERROR("Command buffer %s is not in recording state (%s)", command_buffer->name, state_to_str(command_buffer->state));
        return FALSE;
    }

    VkResult result = vkEndCommandBuffer(command_buffer->handle);
    if (result != VK_SUCCESS) {
        LOG_ERROR("Failed to end command buffer %s: %s", command_buffer->name, vk_result_to_string(result));
        return FALSE;
    }

    command_buffer->state = COMMAND_BUFFER_STATE_ENDED;

    return TRUE;
}

/**
 * @brief Indicates that this command buffer has been submitted.
 *
 * @note This function has no effect other than validating order of operations.
 *
 * @param[in] state The vulkan state.
 * @param[in] command_buffer The command buffer.
 */
void vulkan_command_buffer_submitted(vulkan_state *state, vulkan_command_buffer *command_buffer) {
    if (command_buffer->state != COMMAND_BUFFER_STATE_ENDED) {
        LOG_ERROR("Command buffer %s is not in ended state (%s)", command_buffer->name, state_to_str(command_buffer->state));
        return;
    }

    command_buffer->state = COMMAND_BUFFER_STATE_SUBMITTED;
}

/**
 * @brief Resets a command buffer.
 *
 * @param[in] state The vulkan state.
 * @param[in] command_buffer The command buffer to reset.
 *
 * @retval TRUE Success
 * @retval FALSE Failure
 */
b8 vulkan_command_buffer_reset(vulkan_state *state, vulkan_command_buffer *command_buffer) {
    if (command_buffer->state != COMMAND_BUFFER_STATE_SUBMITTED && command_buffer->state != COMMAND_BUFFER_STATE_READY) {
        LOG_ERROR(
            "Command buffer %s is not in a reset-able state (%s)", command_buffer->name, state_to_str(command_buffer->state));
        return FALSE;
    }

    VkResult result = vkResetCommandBuffer(command_buffer->handle, 0);
    if (result != VK_SUCCESS) {
        LOG_ERROR("Failed to reset command buffer %s: %s", command_buffer->name, vk_result_to_string(result));
        return FALSE;
    }

    command_buffer->state = COMMAND_BUFFER_STATE_READY;

    return TRUE;
}

/**
 * @brief Allocates and begins a single-use command buffer.
 *
 * @param[in] state The vulkan state.
 * @param[in] pool The pool from which the command buffer should be allocated.
 * @param[in] name The name of the command buffer (may be used for debugging purposes).
 * @param[out] command_buffer The command buffer.
 *
 * @retval TRUE Success
 * @retval FALSE Failure
 */
b8 vulkan_command_buffer_begin_single_use(vulkan_state *state,
                                          VkCommandPool pool,
                                          const char *name,
                                          vulkan_command_buffer *command_buffer) {
    if (!vulkan_command_buffer_alloc(state, pool, name, TRUE, command_buffer)) {
        return FALSE;
    }

    return vulkan_command_buffer_begin(state, command_buffer, TRUE, FALSE, FALSE);
}

/**
 * @brief Ends a single-use command buffer, submits it and frees it.
 *
 * @param[in] state The vulkan state.
 * @param[in] pool The pool from which the command buffer was allocated.
 * @param[in] command_buffer The command buffer.
 * @param[in] queue The queue to submit the command buffer to.
 *
 * @retval TRUE Success
 * @retval FALSE Failure
 */
b8 vulkan_command_buffer_end_single_use(vulkan_state *state,
                                        VkCommandPool pool,
                                        vulkan_command_buffer *command_buffer,
                                        VkQueue queue) {
    if (!vulkan_command_buffer_end(state, command_buffer)) {
        return FALSE;
    }

    VkSubmitInfo submit_info = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &command_buffer->handle,
    };

    VkResult result = vkQueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE);
    if (result != VK_SUCCESS) {
        LOG_ERROR("Failed to submit command buffer %s: %s", command_buffer->name, vk_result_to_string(result));
        return FALSE;
    }

    result = vkQueueWaitIdle(queue);
    if (result != VK_SUCCESS) {
        LOG_ERROR("Failed to wait for command buffer %s: %s", command_buffer->name, vk_result_to_string(result));
        return FALSE;
    }

    vulkan_command_buffer_submitted(state, command_buffer);
    vulkan_command_buffer_free(state, pool, command_buffer);

    return TRUE;
}
