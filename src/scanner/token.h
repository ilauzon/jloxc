#pragma once
#include "../arena_allocator/allocator.h"
#include "tokentype.h"
#include <stddef.h>

/**
 * A literal value, used in struct `Token`.
 */
typedef struct Literal {
    /** A pointer to a function to convert the literal to a string. */
    char const *(*to_string)(struct Literal const *);
    /** A pointer to the start of the data in the literal. */
    void const *value;
} Literal;

/**
 * A language token.
 */
typedef struct {
    /** A null-terminated string containing the string representation of the
     * token.
     */
    char const *const lexeme;
    /** Blob containing contents of the token if it is a literal. */
    Literal const *const literal;
    /** The line of source code that the token appears on. */
    unsigned int line;
    /** The type of the token. */
    enum TokenType const type;
} Token;

Literal *token_literal_init_double(ArenaAllocator *allocator, double value);
Literal *token_literal_init_string(ArenaAllocator *allocator,
                                   char const *const value);

Token *token_init(ArenaAllocator *allocator, enum TokenType const type,
                  char const *const lexeme, Literal const *const literal,
                  unsigned int const line);
