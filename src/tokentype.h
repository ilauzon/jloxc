#pragma once
#define TOKENTYPE_ENUM_LIST(X)                                                 \
    X(LEFT_PAREN)                                                              \
    X(RIGHT_PAREN)                                                             \
    X(LEFT_BRACE)                                                              \
    X(RIGHT_BRACE)                                                             \
    X(COMMA)                                                                   \
    X(DOT)                                                                     \
    X(MINUS)                                                                   \
    X(PLUS)                                                                    \
    X(SEMICOLON)                                                               \
    X(SLASH)                                                                   \
    X(STAR)                                                                    \
    X(QUESTION)                                                                \
    X(COLON)                                                                   \
    X(BANG)                                                                    \
    X(BANG_EQUAL)                                                              \
    X(EQUAL)                                                                   \
    X(EQUAL_EQUAL)                                                             \
    X(GREATER)                                                                 \
    X(GREATER_EQUAL)                                                           \
    X(LESS)                                                                    \
    X(LESS_EQUAL)                                                              \
    X(IDENTIFIER)                                                              \
    X(STRING)                                                                  \
    X(NUMBER)                                                                  \
    X(AND)                                                                     \
    X(CLASS)                                                                   \
    X(ELSE)                                                                    \
    X(FALSE)                                                                   \
    X(FUN)                                                                     \
    X(FOR)                                                                     \
    X(IF)                                                                      \
    X(NIL)                                                                     \
    X(OR)                                                                      \
    X(PRINT)                                                                   \
    X(RETURN)                                                                  \
    X(SUPER)                                                                   \
    X(THIS)                                                                    \
    X(TRUE)                                                                    \
    X(VAR)                                                                     \
    X(WHILE)                                                                   \
    X(EOF)

enum TokenType {
#define GENERATE_ENUM(name) TokenType##_##name,
    TOKENTYPE_ENUM_LIST(GENERATE_ENUM)
#undef GENERATE_ENUM
};

char const *tokentype_to_string(enum TokenType t);

enum TokenType tokentype_from_string(char const *const string);
