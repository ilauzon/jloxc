#pragma once
#include "token.h"

/**
 * A token scanner.
 */
typedef struct {
    /** A null-terminated string of source code. */
    char const *const source;
    /** The first character in the lexeme being scanned. */
    size_t start;
    /** The character currently being considered in scanning. */
    size_t current;
    /** The source line that `current` is on. */
    int line;
    /** Pointer to the list of tokens that have been scanned so far. */
    Token *tokens;
    /** The current number of elements in `tokens`. */
    size_t tokens_len;
    /** The number of tokens of allocated memory in `tokens`. */
    size_t tokens_mem_size;
} Scanner;

Scanner *scanner_init(char const *const source);
Token *scanner_scan_tokens(Scanner *const scanner,
                           size_t *const token_list_size);
