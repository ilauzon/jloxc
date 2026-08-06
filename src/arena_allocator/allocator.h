#pragma once
#include <stddef.h>

static int const AUTO_BLOCK_SIZE = 1024;

typedef struct {
    void *start;
    size_t size;
} ArenaBlock;

typedef struct {
    int offset;
    int marked_block;
} ArenaMark;

typedef struct ArenaAllocator {
    ArenaBlock *blocks;
    int block_count;
    ArenaMark mark;
    ArenaMark head;
} ArenaAllocator;

ArenaAllocator *arena_init();

void *arena_allocate(ArenaAllocator *allocator, size_t bytes);

char const *arena_strdup(ArenaAllocator *allocator, char const *s);

void arena_destroy(ArenaAllocator *allocator);

void arena_mark(ArenaAllocator *allocator);

void arena_destroy_until_mark(ArenaAllocator *allocator);
