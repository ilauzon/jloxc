#include "tokentype.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    void *value;
    size_t value_size;
    char *(*to_string)(void const *);
} Literal;

typedef struct {
    char *lexeme;
    Literal *literal;
    unsigned int line;
    enum TokenType type;
} Token;

static inline char *token_to_string(Token const *const token) {
    int const max_size_to_string = 255;
    char *line = calloc(1, max_size_to_string);
    if (!line) {
        perror("memory allocation failed");
        return NULL;
    }
    snprintf(line, max_size_to_string, "%s %s %s",
             tokentype_to_string(token->type), token->lexeme,
             token->literal->to_string(token->literal->value));
    return line;
}

static inline Token *token_init(enum TokenType const type,
                                char const *const lexeme,
                                unsigned int const line,
                                void const *const literal_value,
                                size_t const literal_size,
                                char *(*literal_to_string)(void const *)) {
    Token *token = calloc(1, sizeof(Token));
    if (!token) {
        perror("memory allocation failed");
        return NULL;
    }

    Literal *literal = calloc(1, sizeof(Literal));
    if (!literal) {
        perror("memory allocation failed");
        return NULL;
    }
    if (literal_value && literal_to_string) {
        // memcpy needed instead of foo* = bar* because you can't do that with
        // void pointers
        memcpy(literal->value, literal_value, literal_size);
        literal->to_string = literal_to_string;
    }
    token->literal = literal;

    *token->lexeme = *lexeme;
    token->line = line;
    token->type = type;
    return token;
}

static inline char *eof_to_string(void const *const ignored) { return "<EOF>"; }
static inline Token *token_eof(int line) {
    Token *token = token_init(TokenType_EOF, "", line, "<EOF>", sizeof(char *),
                              eof_to_string);
    return token;
}
