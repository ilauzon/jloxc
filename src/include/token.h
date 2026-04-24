#include "tokentype.h"
#include <stdio.h>
#include <stdlib.h>

typedef char const *LiteralToStringFunc(void const *);

struct Literal {
    void const *const value;
    LiteralToStringFunc *literal_to_string;
};

struct Token {
    char const *const lexeme;
    struct Literal const *const literal;
    unsigned int const line;
    enum TokenType const type;
};

inline char *token_to_string(struct Token const *const token) {
    int const max_size_to_string = 255;
    char *line = malloc(max_size_to_string);
    snprintf(line, max_size_to_string, "%s %s %s",
             tokentype_to_string(token->type), token->lexeme,
             token->literal->literal_to_string(token->literal->value));
    return line;
}
