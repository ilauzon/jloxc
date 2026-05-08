#pragma once
#include "../token.h"

typedef struct ExprBinary ExprBinary;
typedef struct ExprUnary ExprUnary;
typedef struct ExprLiteral ExprLiteral;
typedef struct ExprGrouping ExprGrouping;

typedef struct ExprVisitor {
    void *(*visit_binary)(ExprBinary const *expr);
    void *(*visit_unary)(ExprUnary const *expr);
    void *(*visit_literal)(ExprLiteral const *expr);
    void *(*visit_grouping)(ExprGrouping const *expr);
} ExprVisitor;

typedef struct Expr {
    void *(*accept)(void const *data, ExprVisitor const *visitor);
    char const *(*to_string)(void const *self);
} Expr;

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
    Literal const *literal;
} ExprLiteral;
ExprLiteral *expr_init_literal(Literal const *literal);

typedef struct ExprGrouping {
    Expr super;
    Expr const *expression;
} ExprGrouping;
ExprGrouping *expr_init_grouping(Expr const *expression);
