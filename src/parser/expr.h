#pragma once
#include "../token.h"
#include <stdbool.h>

#define EXPRTYPE_ENUM_LIST(X)                                                  \
    X(UNARY)                                                                   \
    X(BINARY)                                                                  \
    X(TERNARY)                                                                 \
    X(LITERAL)                                                                 \
    X(GROUPING)

enum ExprType {
#define GENERATE_ENUM(name) ExprType##_##name,
    EXPRTYPE_ENUM_LIST(GENERATE_ENUM)
#undef GENERATE_ENUM
};

typedef struct Expr Expr;

typedef struct ExprTernary {
    Expr const *left;
    Token const *operator_left;
    Expr const *middle;
    Token const *operator_right;
    Expr const *right;
} ExprTernary;
Expr *expr_init_ternary(Expr const *left, Token const *operator_left,
                        Expr const *middle, Token const *operator_right,
                        Expr const *right);

typedef struct ExprBinary {
    Expr const *left;
    Token const *operator;
    Expr const *right;
} ExprBinary;
Expr *expr_init_binary(Expr const *left, Token const *operator,
                       Expr const * right);

typedef struct ExprUnary {
    Token const *operator;
    Expr const *right;
} ExprUnary;
Expr *expr_init_unary(Token const *operator, Expr const * right);

typedef struct ExprLiteral {
    enum TokenType type;
    void const *value;
} ExprLiteral;
Expr *expr_init_literal(Token const *const token);

typedef struct ExprGrouping {
    Expr const *expression;
} ExprGrouping;
Expr *expr_init_grouping(Expr const *expression);

bool expr_type_is_literal(enum TokenType const type);

Expr *expr_init_missing(void);

typedef struct Expr {
    enum ExprType type;
    union {
        ExprTernary ternary;
        ExprBinary binary;
        ExprUnary unary;
        ExprLiteral literal;
        ExprGrouping grouping;
    } value;
} Expr;

char const *expr_to_string(Expr const *const expr);
