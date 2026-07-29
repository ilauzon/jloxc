#include "allocator.h"
#include <stdlib.h>
#include <string.h>

ArenaAllocator *arena_init(size_t initial_size) {
    // TODO add check for initial_size being greater than 0
    if (initial_size == 0) {
        initial_size = 1;
    }
    ArenaAllocator *allocator = calloc(1, sizeof(ArenaAllocator));
    allocator->arena = calloc(1, initial_size);
    allocator->arena_size = initial_size;
    return allocator;
}

static void expand_if_needed(ArenaAllocator *allocator, size_t bytes_to_add) {
    while (allocator->head + bytes_to_add >= allocator->arena_size) {
        allocator->arena_size *= 2;
        allocator->arena = realloc(allocator->arena, allocator->arena_size);
    }
}

void *arena_allocate(ArenaAllocator *allocator, size_t bytes) {
    expand_if_needed(allocator, bytes);
    void *ptr = (char *)(allocator->arena) + allocator->head;
    allocator->head += bytes;
    return ptr;
}

void arena_destroy(ArenaAllocator *allocator) { free(allocator->arena); }

void arena_mark(ArenaAllocator *allocator) {
    allocator->mark = allocator->head;
}

void arena_destroy_until_mark(ArenaAllocator *allocator) {
    // zero out memory as a precaution for future allocations expecting zeroed
    // memory
    void *ptr = (char *)(allocator->arena) + allocator->mark;
    memset(ptr, 0, allocator->head - allocator->mark);

    allocator->head = allocator->mark;
}
