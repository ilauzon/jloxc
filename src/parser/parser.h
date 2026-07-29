#include "expr.h"
#include <stdlib.h>

typedef struct {
    Token const *tokens;
    size_t tokens_len;
    int current;
    ArenaAllocator *allocator;
    bool panicking;
} Parser;

Stmt **parser_parse(ArenaAllocator *allocator, Token const *const tokens,
                    size_t const tokens_len, size_t *return_length_ptr);
