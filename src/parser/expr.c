/*
 * Contains initialization functions for all types of Exprs.
 */

#include "expr.h"
#include "../errors/errorhandler.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

/* ternary expressions */

Expr *expr_init_ternary(ArenaAllocator *allocator, Expr const *left,
                        Token const *operator_left, Expr const *middle,
                        Token const *operator_right, Expr const *right) {
    Expr *e = arena_allocate(allocator, sizeof(Expr));
    e->type = ExprType_TERNARY;
    if (operator_left->type == TokenType_QUESTION &&
        operator_right->type == TokenType_COLON) {
        e->value.ternary.type = ExprTernaryType_CONDITIONAL;
    } else {
        errorhandler_printerror(e->line,
                                "attempt to create ternary expression failed "
                                "due to invalid TokenType(s)");
        return NULL;
    }
    e->value.ternary.left = left;
    e->value.ternary.middle = middle;
    e->value.ternary.right = right;
    return e;
}

/* binary expressions */

Expr *expr_init_binary(ArenaAllocator *allocator, Expr const *left,
                       Token const *const operator, Expr const *right) {
    Expr *e = arena_allocate(allocator, sizeof(Expr));
    e->type = ExprType_BINARY;
    e->value.binary.left = left;
    e->value.binary.right = right;
    switch (operator->type) {
    case TokenType_COMMA:
        e->value.binary.type = ExprBinaryType_COMMA;
        break;
    case TokenType_PLUS:
        e->value.binary.type = ExprBinaryType_PLUS;
        break;
    case TokenType_MINUS:
        e->value.binary.type = ExprBinaryType_MINUS;
        break;
    case TokenType_SLASH:
        e->value.binary.type = ExprBinaryType_SLASH;
        break;
    case TokenType_STAR:
        e->value.binary.type = ExprBinaryType_STAR;
        break;
    case TokenType_GREATER:
        e->value.binary.type = ExprBinaryType_GREATER;
        break;
    case TokenType_GREATER_EQUAL:
        e->value.binary.type = ExprBinaryType_GREATER_EQUAL;
        break;
    case TokenType_LESS_EQUAL:
        e->value.binary.type = ExprBinaryType_LESS_EQUAL;
        break;
    case TokenType_LESS:
        e->value.binary.type = ExprBinaryType_LESS;
        break;
    case TokenType_BANG_EQUAL:
        e->value.binary.type = ExprBinaryType_BANG_EQUAL;
        break;
    case TokenType_EQUAL_EQUAL:
        e->value.binary.type = ExprBinaryType_EQUAL_EQUAL;
        break;
    default:
        errorhandler_printerror(e->line,
                                "attempt to create binary expression failed "
                                "due to invalid TokenType");
        return NULL;
    }
    return e;
}

/* unary expressions */

Expr *expr_init_unary(ArenaAllocator *allocator, Token const *const operator,
                      Expr const *const right) {
    Expr *e = arena_allocate(allocator, sizeof(Expr));
    e->type = ExprType_UNARY;
    e->line = operator->line;
    switch (operator->type) {
    case TokenType_MINUS:
        e->value.unary.type = ExprUnaryType_MINUS;
        break;
    case TokenType_BANG:
        e->value.unary.type = ExprUnaryType_BANG;
        break;
    default:
        errorhandler_printerror(e->line,
                                "attempt to create unary expression failed "
                                "due to invalid TokenType");
        return NULL;
    }
    e->value.unary.right = right;
    return e;
}

/* variable expressions */

Expr *expr_init_var(ArenaAllocator *allocator, Token const *const name) {
    Expr *e = arena_allocate(allocator, sizeof(Expr));
    e->type = ExprType_VAR;
    e->line = name->line;
    e->value.var.name = name->lexeme;
    return e;
}

/* literals */

Expr *expr_init_literal(ArenaAllocator *allocator, Token const *const token) {
    Expr *e = arena_allocate(allocator, sizeof(Expr));
    e->type = ExprType_LITERAL;
    e->line = token->line;

    switch (token->type) {
    case TokenType_MISSING:
        e->value.literal.type = ExprLiteralType_MISSING;
        break;
    case TokenType_NIL:
        e->value.literal.type = ExprLiteralType_NIL;
        break;
    case TokenType_TRUE:
        e->value.literal.type = ExprLiteralType_TRUE;
        break;
    case TokenType_FALSE:
        e->value.literal.type = ExprLiteralType_FALSE;
        break;
    case TokenType_IDENTIFIER:
        e->value.literal.type = ExprLiteralType_IDENTIFIER;
        // TODO fix this so that it works with the arena allocator, i.e. copy
        // the value instead of its pointer.
        e->value.literal.value = token->lexeme;
        break;
    case TokenType_STRING:
        e->value.literal.type = ExprLiteralType_STRING;
        e->value.literal.value = token->literal->value;
        break;
    case TokenType_NUMBER:
        e->value.literal.type = ExprLiteralType_NUMBER;
        e->value.literal.value = token->literal->value;
        break;
    default:
        errorhandler_printerror(e->line,
                                "attempt to create literal expression failed "
                                "due to invalid TokenType");
        return NULL;
    }

    return e;
}

Expr *expr_init_missing(ArenaAllocator *allocator) {
    Token *token = token_init(TokenType_MISSING, NULL, NULL, 0);
    Expr *literal = expr_init_literal(allocator, token);
    free(token);
    return literal;
}

bool expr_token_is_literal(Token const *const token) {
    switch (token->type) {
    case TokenType_MISSING:
    case TokenType_NIL:
    case TokenType_TRUE:
    case TokenType_FALSE:
    case TokenType_IDENTIFIER:
    case TokenType_STRING:
    case TokenType_NUMBER:
        return true;
    default:
        return false;
    }
}

/* groupings */

Expr *expr_init_grouping(ArenaAllocator *allocator,
                         Expr const *const expression) {
    Expr *e = arena_allocate(allocator, sizeof(Expr));
    e->type = ExprType_GROUPING;
    e->value.grouping.expression = expression;
    return e;
}

/* statements */

Stmt *stmt_expr_init(ArenaAllocator *allocator, Expr *expression) {
    Stmt *stmt = arena_allocate(allocator, sizeof(Stmt));
    stmt->type = StmtType_EXPR;
    stmt->value.expr.expression = expression;
    return stmt;
}

Stmt *stmt_print_init(ArenaAllocator *allocator, Expr *expression) {
    Stmt *stmt = arena_allocate(allocator, sizeof(Stmt));
    stmt->type = StmtType_PRINT;
    stmt->value.print.expression = expression;
    return stmt;
}

Stmt *stmt_var_init(ArenaAllocator *allocator, char const *name,
                    Expr *expression) {
    Stmt *stmt = arena_allocate(allocator, sizeof(Stmt));
    stmt->type = StmtType_VAR;
    stmt->value.var.name = name;
    stmt->value.var.initializer = expression;
    return stmt;
}
