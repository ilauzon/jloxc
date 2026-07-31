#pragma once
#include <stddef.h>

typedef struct ArenaAllocator {
    void *arena;
    size_t arena_size;
    size_t head;
    size_t mark;
} ArenaAllocator;

ArenaAllocator *arena_init(size_t initial_size);

void *arena_allocate(ArenaAllocator *allocator, size_t bytes);

char const *arena_strdup(ArenaAllocator *allocator, char const *s);

void arena_destroy(ArenaAllocator *allocator);

void arena_mark(ArenaAllocator *allocator);

void arena_destroy_until_mark(ArenaAllocator *allocator);
