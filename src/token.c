#include "token.h"

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

/**
 * @brief Create a new `Literal`.
 *
 * @param value The literal's location in memory.
 * @param value_size The size of the literal in bytes.
 * @param to_string The function to convert the literal to a human-readable
 * string.
 * @return A pointer to the literal that must be freed.
 */
Literal *token_literal_init(void const *const value, size_t const value_size,
                            LiteralToString const to_string) {
    Literal const literal_init = {
        .to_string = to_string,
        .value = value,
        .value_size = value_size,
    };

    Literal *literal = malloc(sizeof(Literal));
    if (!literal) {
        perror("memory allocation failed");
        return NULL;
    }

    memcpy(literal, &literal_init, sizeof(Literal));
    return literal;
}
