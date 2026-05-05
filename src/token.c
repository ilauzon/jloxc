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
Token *token_init(enum TokenType const type, char const *const lexeme,
                  Literal const *const literal, unsigned int const line) {
    Token token_init = {
        .line = line,
        .literal = literal,
        .lexeme = lexeme,
        .type = type,
    };

    Token *token = malloc(sizeof(Token));
    if (!token) {
        perror("memory allocation failed");
        return NULL;
    }

    memcpy(token, &token_init, sizeof(Token));
    return token;
}

/* String literals */

char const *token_literal_string_to_string(Literal const *const it) {
    return it->value;
}

Literal *token_literal_init_string(char const *const value) {
    Literal *literal = malloc(sizeof(Literal));
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

Literal *token_literal_init_double(double value) {
    Literal *literal = malloc(sizeof(Literal));
    double *value_ptr = malloc(sizeof(value));
    memcpy(value_ptr, &value, sizeof(value));
    literal->value = value_ptr;
    literal->to_string = token_literal_double_to_string;
    return literal;
}

/* Integer literals */

char const *token_literal_int_to_string(Literal const *const it) {
    char *string = malloc(50);
    snprintf(string, 50, "%d", *(int *)(it->value));
    return string;
}
Literal *token_literal_init_int(int value) {
    Literal *literal = malloc(sizeof(Literal));
    int *value_ptr = malloc(sizeof(value));
    memcpy(value_ptr, &value, sizeof(value));
    literal->value = value_ptr;
    literal->to_string = token_literal_int_to_string;
    return literal;
}
