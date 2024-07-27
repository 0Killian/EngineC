#include "memory.h"
#include "platform/platform.h"
#include "core/log.h"

#include <string.h>

// TODO: The allocation functions provided by the platforms are very "generic" in nature and are not very efficient for our use
// case. We should avoid tham at all cost, by pre-allocating a large memory region and then only allocating from it using our own
// allocators -> Optimize this system after the first phase of the project

typedef struct region_header {
    u64 size;
    u64 allocation_offset;
    u64 allocation_size;
    memory_tag tag;

    #ifdef DEBUG
        struct region_header* prev;
        struct region_header* next;
        // NOTE: If the caller is a generic function (like an allocator), we can modify this system to store a call stack
        // specifically for this function, in order to better locate the memory leak
        void* caller;
    #endif
} region_header;

typedef struct memory_state {
    u64 allocation_count[MEMORY_TAG_MAX_TAGS];
    u64 allocated_size[MEMORY_TAG_MAX_TAGS];

    #ifdef DEBUG
        region_header* regions_list_head[MEMORY_TAG_MAX_TAGS];
        region_header* regions_list_tail[MEMORY_TAG_MAX_TAGS];
    #endif
} memory_state;

static const char* TAG_LABELS[MEMORY_TAG_MAX_TAGS] = {
    "UNKNOWN",
    "DYNARRAY",
    "HASHTABLE",
    "ENGINE",
    "PLATFORM",
    "STRING",
    "RENDERER"
};

static memory_state* state = NULL;

/**
 * @brief Initializes the memory system.
 * 
 * @note Unlike other systems, ths function will allocate the needed memory itself. As such, it must be the first system to be
 * initialized.
 * 
 * @retval TRUE Success
 * @retval FALSE Failure
 */
b8 mem_init() {
    state = (memory_state*)platform_allocate(sizeof(memory_state));
    mem_zero(state, sizeof(memory_state));

    return TRUE;
}

/**
 * @brief Deinitializes the memory system.
 */
void mem_deinit() {
    // Check if any allocations are left, and warn about them
    for(u64 i = 0; i < MEMORY_TAG_MAX_TAGS; i++) {
        if(state->allocation_count[i] != 0) {
            LOG_WARN("Memory leak of %llu bytes (%llu allocations) in tag %s",
                state->allocated_size[i], state->allocation_count[i], TAG_LABELS[i]);

            #ifdef DEBUG
                region_header* current = state->regions_list_head[i];
                while(current != NULL) {
                    LOG_WARN("  0x%p (%llu bytes) allocated by 0x%p", current, current->size, current->caller);

                    current = current->next;
                }
            #endif
        }
    }

    platform_free(state);
}

static void* mem_alloc_aligned_with_caller(memory_tag tag, u64 size, u64 alignment, void* caller) {
    if (tag == MEMORY_TAG_UNKNOWN) {
        LOG_WARN("An allocation with an unknown tag was requested. Tag this allocation accordingly.");
    }

    // Calculate the maximum size of the region, taking into account the alignment and the header
    u64 region_size = size + sizeof(region_header) + alignment;

    // Allocate the region
    void* allocation = platform_allocate(region_size);
    if(allocation == NULL) {
        LOG_FATAL("Failed to allocate %llu bytes in tag %s", region_size);
        return NULL;
    }

    // Initialize the region header in the correct location
    void* region = (void*)(((u64)allocation + sizeof(region_header) + alignment - 1) / alignment * alignment);
    region_header* header = (region_header*)((u64)region - sizeof(region_header));
    header->size = size;
    header->allocation_offset = (u64)header - (u64)allocation;
    header->allocation_size = region_size;
    header->tag = tag;

    #ifdef DEBUG
        header->caller = caller;
        header->prev = state->regions_list_tail[tag];
        header->next = NULL;

        // Update the linked list
        if(state->regions_list_tail[tag] != NULL) {
            state->regions_list_tail[tag]->next = header;
        } else {
            state->regions_list_head[tag] = header;
        }
        state->regions_list_tail[tag] = header;
    #endif

    // Update the statistics
    state->allocation_count[tag]++;
    state->allocated_size[tag] += region_size;

    return region;
}

/**
 * @brief Allocates a memory region of the given size and tag.
 * 
 * @param tag The tag of the memory to allocate.
 * @param size The size of the memory to allocate.
 * @return A pointer to the allocated memory region.
 */
API void* mem_alloc(memory_tag tag, u64 size) {
    return mem_alloc_aligned_with_caller(tag, size, 1, platform_get_caller());
}

/**
 * @brief Allocates an aligned memory region of the given size and tag.
 * 
 * @param tag The tag of the memory to allocate.
 * @param size The size of the memory to allocate.
 * @param alignment The alignment of the memory to allocate.
 * @return A pointer to the allocated memory region.
 */
API void* mem_alloc_aligned(memory_tag tag, u64 size, u64 alignment) {
    return mem_alloc_aligned_with_caller(tag, size, alignment, platform_get_caller());
}

/**
 * @brief Frees the given memory region.
 * 
 * @param ptr The memory region to free.
 */
API void mem_free(void* ptr) {
    // TODO: We should find a way to make sure that the given pointer points directly after a valid header
    region_header* header = (region_header*)((u64)ptr - sizeof(region_header));

    // Update the linked list
    #ifdef DEBUG
        if(header->prev != NULL) {
            header->prev->next = header->next;
        } else {
            state->regions_list_head[header->tag] = header->next;
        }

        if(header->next != NULL) {
            header->next->prev = header->prev;
        } else {
            state->regions_list_tail[header->tag] = header->prev;
        }
    #endif

    // Update the statistics
    state->allocation_count[header->tag]--;
    state->allocated_size[header->tag] -= header->allocation_size;

    // Free the region
    platform_free(header - header->allocation_offset);
}

/**
 * @brief Zeroes out the given memory region.
 * 
 * @param ptr The memory region to zero.
 * @param size The size of the memory region to zero.
 */
API void mem_zero(void* ptr, u64 size) {
    mem_set(ptr, 0, size);
}

/**
 * @brief Copies the given memory region.
 * 
 * @param dst The destination memory region.
 * @param src The source memory region.
 * @param size The size of the memory region to copy.
 */
API void mem_copy(void* dst, const void* src, u64 size) {
    memcpy(dst, src, size);
}

/**
 * @brief Moves the given memory region.
 * 
 * @param dst The destination memory region.
 * @param src The source memory region.
 * @param size The size of the memory region to move.
 */
API void mem_move(void* dst, const void* src, u64 size) {
    memmove(dst, src, size);
}

/**
 * @brief Fills the given memory region with the given value.
 * 
 * @param ptr The memory region to fill.
 * @param value The value to fill the memory region with.
 * @param size The size of the memory region to fill.
 */
API void mem_set(void* ptr, u8 value, u64 size) {
    memset(ptr, value, size);
}
