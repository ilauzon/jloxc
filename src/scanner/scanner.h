#pragma once
#include "../arena_allocator/allocator.h"
#include "token.h"
#include <stdlib.h>

Token const *const *scanner_scan_tokens(ArenaAllocator *allocator,
                                        char const *const source,
                                        size_t *const token_list_size);
