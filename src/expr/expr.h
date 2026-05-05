#pragma once
#include "../token.h"

typedef struct ExprBinary ExprBinary;
typedef struct ExprUnary ExprUnary;
typedef struct ExprLiteral ExprLiteral;
typedef struct ExprGrouping ExprGrouping;

typedef struct ExprVisitor {
    void *(*visit_binary)(ExprBinary *expr);
    void *(*visit_unary)(ExprUnary *expr);
    void *(*visit_literal)(ExprLiteral *expr);
    void *(*visit_grouping)(ExprGrouping *expr);
} ExprVisitor;

typedef struct Expr {
    void *(*accept)(void *data, ExprVisitor *visitor);
    char const *(*to_string)(void *self);
} Expr;

typedef struct ExprBinary {
    Expr super;
    Expr *left;
    Token *operator;
    Expr *right;
} ExprBinary;
ExprBinary *expr_init_binary(Expr *left, Token *operator, Expr * right);

typedef struct ExprUnary {
    Expr super;
    Token *operator;
    Expr *right;
} ExprUnary;
ExprUnary *expr_init_unary(Token *operator, Expr * right);

typedef struct ExprLiteral {
    Expr super;
    Literal *literal;
} ExprLiteral;
ExprLiteral *expr_init_literal(Literal *literal);

typedef struct ExprGrouping {
    Expr super;
    Expr *expression;
} ExprGrouping;
ExprGrouping *expr_init_grouping(Expr *expression);
