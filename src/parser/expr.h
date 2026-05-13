#pragma once
#include "../token.h"
#include <stdbool.h>

typedef struct ExprBinary ExprBinary;
typedef struct ExprUnary ExprUnary;
typedef struct ExprLiteral ExprLiteral;
typedef struct ExprGrouping ExprGrouping;

typedef struct Expr {
    // Get a free-able string representation of the expression.
    char const *(*to_string)(void const *self);
} Expr;

typedef struct ExprTernary {
    Expr super;
    Expr const *left;
    Token const *operator_left;
    Expr const *middle;
    Token const *operator_right;
    Expr const *right;
} ExprTernary;
ExprTernary *expr_init_ternary(Expr const *left, Token const *operator_left,
                               Expr const *middle, Token const *operator_right,
                               Expr const *right);

typedef struct ExprBinary {
    Expr super;
    Expr const *left;
    Token const *operator;
    Expr const *right;
} ExprBinary;
ExprBinary *expr_init_binary(Expr const *left, Token const *operator,
                             Expr const * right);

typedef struct ExprUnary {
    Expr super;
    Token const *operator;
    Expr const *right;
} ExprUnary;
ExprUnary *expr_init_unary(Token const *operator, Expr const * right);

typedef struct ExprLiteral {
    Expr super;
    enum TokenType type;
    void const *value;
} ExprLiteral;
ExprLiteral *expr_init_literal(Token const *const token);

typedef struct ExprGrouping {
    Expr super;
    Expr const *expression;
} ExprGrouping;
ExprGrouping *expr_init_grouping(Expr const *expression);

bool expr_type_is_literal(enum TokenType const type);
