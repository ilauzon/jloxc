#include "token.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Create a new token.
 *
 * @param type The type of the new token.
 * @param lexeme The lexeme of the new token.
 * @param line The line that the token appears on.
 * @param literal A pointer to the literal value of the token. May be null if
 * the token is not a literal.
 * @return A pointer to the new token that must be freed.
 */
Token *token_init(ArenaAllocator *allocator, enum TokenType const type,
                  char const *const lexeme, Literal const *const literal,
                  unsigned int const line) {
    Token token_init = {
        .line = line,
        .literal = literal,
        .lexeme = lexeme,
        .type = type,
    };

    Token *token = arena_allocate(allocator, sizeof(Token));

    memcpy(token, &token_init, sizeof(Token));
    return token;
}

/* String literals */

char const *token_literal_string_to_string(Literal const *const it) {
    size_t len = strlen(it->value);
    char *str = calloc(1, len + 1);
    strncpy(str, it->value, len);
    str[len] = '\0';
    return str;
}

Literal *token_literal_init_string(ArenaAllocator *allocator,
                                   char const *const value) {
    Literal *literal = arena_allocate(allocator, sizeof(Literal));
    literal->value = value;
    literal->to_string = token_literal_string_to_string;
    return literal;
}

/* Floating-point literals */

char const *token_literal_double_to_string(Literal const *const it) {
    char *string = malloc(50);
    snprintf(string, 50, "%f", *(double *)(it->value));
    return string;
}

Literal *token_literal_init_double(ArenaAllocator *allocator, double value) {
    Literal *literal = arena_allocate(allocator, sizeof(Literal));
    double *value_ptr = arena_allocate(allocator, sizeof(value));
    memcpy(value_ptr, &value, sizeof(value));
    literal->value = value_ptr;
    literal->to_string = token_literal_double_to_string;
    return literal;
}
