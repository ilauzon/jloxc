#pragma once
#include "../token.h"
#include <stdbool.h>

enum ExprType {
    ExprType_UNARY,
    ExprType_BINARY,
    ExprType_TERNARY,
    ExprType_LITERAL,
    ExprType_GROUPING
};

typedef struct Expr Expr;

typedef struct ExprTernary {
    enum ExprTernaryType {
        ExprTernaryType_CONDITIONAL,
    } type;
    Expr const *left;
    Expr const *middle;
    Expr const *right;
} ExprTernary;
Expr *expr_init_ternary(Expr const *left, Token const *operator_left,
                        Expr const *middle, Token const *operator_right,
                        Expr const *right);

typedef struct ExprBinary {
    enum ExprBinaryType {
        ExprBinaryType_PLUS,
        ExprBinaryType_MINUS,
        ExprBinaryType_SLASH,
        ExprBinaryType_STAR,
        ExprBinaryType_GREATER,
        ExprBinaryType_GREATER_EQUAL,
        ExprBinaryType_LESS_EQUAL,
        ExprBinaryType_LESS,
        ExprBinaryType_BANG_EQUAL,
        ExprBinaryType_EQUAL_EQUAL,
    } type;
    Expr const *left;
    Expr const *right;
} ExprBinary;
Expr *expr_init_binary(Expr const *left, Token const *operator,
                       Expr const * right);

typedef struct ExprUnary {
    enum ExprUnaryType {
        ExprUnaryType_MINUS,
        ExprUnaryType_BANG,
    } type;
    Expr const *right;
} ExprUnary;
Expr *expr_init_unary(Token const *operator, Expr const * right);

typedef struct ExprLiteral {
    enum ExprLiteralType {
        ExprLiteralType_MISSING,
        ExprLiteralType_NIL,
        ExprLiteralType_TRUE,
        ExprLiteralType_FALSE,
        ExprLiteralType_IDENTIFIER,
        ExprLiteralType_STRING,
        ExprLiteralType_NUMBER,
    } type;
    void const *value;
} ExprLiteral;
Expr *expr_init_literal(Token const *const token);

bool expr_token_is_literal(Token const *const token);

typedef struct ExprGrouping {
    Expr const *expression;
} ExprGrouping;
Expr *expr_init_grouping(Expr const *expression);

Expr *expr_init_missing(void);

typedef struct Expr {
    enum ExprType type;
    int line;
    union {
        ExprTernary ternary;
        ExprBinary binary;
        ExprUnary unary;
        ExprLiteral literal;
        ExprGrouping grouping;
    } value;
} Expr;

char const *expr_to_string(Expr const *const expr);
