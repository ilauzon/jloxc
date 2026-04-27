#pragma once
#include "tokentype.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef char const *(*LiteralToString)(void const *, size_t const);

/**
 * A literal value, used in struct `Token`.
 */
typedef struct {
    /** A pointer to the start of the data in the literal. */
    void const *const value;
    /** The number of bytes in `value`. Needed for memory copies and
     * allocations. */
    size_t const value_size;
    /** A pointer to a function to convert the literal to a string. */
    LiteralToString const to_string;
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

Literal *token_literal_init(void const *const value, size_t const value_size,
                            LiteralToString const to_string);

Token *token_init(enum TokenType const type, char const *const lexeme,
                  Literal const *const literal, unsigned int const line);
