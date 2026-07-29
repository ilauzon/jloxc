#pragma once
#include "../arena_allocator/allocator.h"
#include "../scanner/token.h"
#include <stdbool.h>
#include <stddef.h>

enum ExprType {
    ExprType_VAR,
    ExprType_UNARY,
    ExprType_BINARY,
    ExprType_TERNARY,
    ExprType_LITERAL,
    ExprType_GROUPING,
};

typedef struct Expr Expr;

typedef struct ExprVar {
    char const *name;
} ExprVar;
Expr *expr_init_var(ArenaAllocator *allocator, Token const *const name);

typedef struct ExprTernary {
    enum ExprTernaryType {
        ExprTernaryType_CONDITIONAL,
    } type;
    Expr const *left;
    Expr const *middle;
    Expr const *right;
} ExprTernary;
Expr *expr_init_ternary(ArenaAllocator *allocator, Expr const *left,
                        Token const *operator_left, Expr const *middle,
                        Token const *operator_right, Expr const *right);

typedef struct ExprBinary {
    enum ExprBinaryType {
        ExprBinaryType_COMMA,
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
Expr *expr_init_binary(ArenaAllocator *allocator, Expr const *left,
                       Token const *operator, Expr const * right);

typedef struct ExprUnary {
    enum ExprUnaryType {
        ExprUnaryType_MINUS,
        ExprUnaryType_BANG,
    } type;
    Expr const *right;
} ExprUnary;
Expr *expr_init_unary(ArenaAllocator *allocator, Token const *operator,
                      Expr const * right);

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
Expr *expr_init_literal(ArenaAllocator *allocator, Token const *const token);
bool expr_token_is_literal(Token const *const token);
Expr *expr_init_missing(ArenaAllocator *allocator);

typedef struct ExprGrouping {
    Expr const *expression;
} ExprGrouping;
Expr *expr_init_grouping(ArenaAllocator *allocator, Expr const *expression);

typedef struct Expr {
    enum ExprType type;
    int line;
    union {
        ExprVar var;
        ExprTernary ternary;
        ExprBinary binary;
        ExprUnary unary;
        ExprLiteral literal;
        ExprGrouping grouping;
    } value;
} Expr;

typedef struct {
    enum StmtType {
        StmtType_EXPR,
        StmtType_PRINT,
        StmtType_VAR,
    } type;
    union {
        struct {
            Expr *expression;
        } expr;
        struct {
            Expr *expression;
        } print;
        struct {
            char const *name;
            /** The statement initializing the variable. Can be null. */
            Expr *initializer;
        } var;
    } value;
} Stmt;

Stmt *stmt_expr_init(ArenaAllocator *allocator, Expr *expression);
Stmt *stmt_print_init(ArenaAllocator *allocator, Expr *expression);
Stmt *stmt_var_init(ArenaAllocator *allocator, char const *name,
                    Expr *expression);
