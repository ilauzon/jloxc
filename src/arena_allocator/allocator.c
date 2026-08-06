#include "allocator.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void create_new_block(ArenaAllocator *allocator, size_t bytes) {
    assert(allocator->head.marked_block < allocator->block_count);
    ArenaBlock block = {
        .start = calloc(1, bytes),
        .size = bytes,
    };
    if (allocator->head.marked_block == allocator->block_count - 1) {
        allocator->block_count++;
        allocator->blocks = realloc(
            allocator->blocks, sizeof(ArenaBlock) * allocator->block_count);

        memcpy(allocator->blocks + allocator->block_count - 1, &block,
               sizeof(ArenaBlock));
        allocator->head.marked_block = allocator->block_count - 1;
    } else {
        allocator->head.marked_block++;
        free(allocator->blocks[allocator->head.marked_block].start);
        memcpy(allocator->blocks + allocator->head.marked_block, &block,
               sizeof(ArenaBlock));
    }
    allocator->head.offset = 0;
}

ArenaAllocator *arena_init() {
    ArenaAllocator *allocator = calloc(1, sizeof(ArenaAllocator));
    ArenaBlock *arena_blocks = calloc(1, sizeof(ArenaBlock));
    arena_blocks[0].start = calloc(1, AUTO_BLOCK_SIZE);
    arena_blocks[0].size = AUTO_BLOCK_SIZE;
    allocator->blocks = arena_blocks;
    allocator->block_count = 1;
    return allocator;
}

void *arena_allocate(ArenaAllocator *allocator, size_t bytes) {
    size_t offset = alignof(max_align_t) - bytes % alignof(max_align_t);
    size_t bytes_with_offset = bytes + offset;
    if (bytes == 0)
        return NULL;
    void *ptr = NULL;

    ArenaBlock current_block = allocator->blocks[allocator->head.marked_block];
    int current_block_head = allocator->head.offset;

    // check if the current block has enough space for this allocation
    if (current_block_head + bytes_with_offset <= current_block.size) {
        // if so, then allocate and return pointer to memory
        ptr = (char *)current_block.start + current_block_head;
    } else {
        // otherwise, allocate a new block of the required size and return
        // pointer to memory
        create_new_block(allocator, bytes_with_offset < AUTO_BLOCK_SIZE
                                        ? AUTO_BLOCK_SIZE
                                        : bytes_with_offset);
        ptr = allocator->blocks[allocator->head.marked_block].start;
    }
    allocator->head.offset += bytes_with_offset;
    return ptr;
}

char const *arena_strdup(ArenaAllocator *allocator, char const *const s) {
    size_t len = strlen(s);
    char *ptr = arena_allocate(allocator, len + 1);
    memcpy(ptr, s, len);
    return ptr;
}

/**
 * @brief Free the arena memory and the allocator itself.
 *
 * Frees allocator->arena and allocator.
 *
 * @param allocator The allocator to free.
 */
void arena_destroy(ArenaAllocator *allocator) {
    for (int i = 0; i < allocator->block_count; ++i) {
        free(allocator->blocks[i].start);
    }
    free(allocator->blocks);
    free(allocator);
}

void arena_mark(ArenaAllocator *allocator) {
    // TODO
}

void arena_destroy_until_mark(ArenaAllocator *allocator) {
    // TODO
}
