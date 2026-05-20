#include "expr.h"
#include "../errorhandler.h"
#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char const *ternary_to_string(Expr const *const expr);
static char const *binary_to_string(Expr const *const expr);
static char const *unary_to_string(Expr const *const expr);
static char const *literal_to_string(Expr const *const expr);
static char const *grouping_to_string(Expr const *const expr);

/**
 * @brief Get a free-able string representation of the expression.
 *
 * @param expr The expression.
 * @return The string representation.
 */
char const *expr_to_string(Expr const *const expr) {
    switch (expr->type) {
    case ExprType_UNARY:
        return unary_to_string(expr);
    case ExprType_BINARY:
        return binary_to_string(expr);
    case ExprType_TERNARY:
        return ternary_to_string(expr);
    case ExprType_LITERAL:
        return literal_to_string(expr);
    case ExprType_GROUPING:
        return grouping_to_string(expr);
    }
    return NULL;
}

static char *ast_parenthesize(char const *const name, int arg_count, ...) {

    size_t total_str_mem_size =
        strlen(name) + 2; // null terminator and starting parenthesis
    char *string = calloc(1, total_str_mem_size);
    string[0] = '(';
    memcpy(string + 1, name, total_str_mem_size);
    string[total_str_mem_size - 1] = '\0';

    va_list exprs;
    va_start(exprs, arg_count);
    for (int i = 0; i < arg_count; ++i) {
        Expr *expr = va_arg(exprs, Expr *);
        char const *expr_str = expr_to_string(expr);
        size_t expr_strlen = strlen(expr_str);
        size_t prev_strlen = total_str_mem_size;
        total_str_mem_size += expr_strlen;
        total_str_mem_size += 1; // +1 for space character
        string = realloc(string, total_str_mem_size);
        string[prev_strlen - 1] = ' ';
        memcpy(string + prev_strlen, expr_str, expr_strlen);
        string[total_str_mem_size - 1] = '\0';
        free((char *)expr_str);
    }
    va_end(exprs);

    // place end parenthesis
    ++total_str_mem_size;
    string = realloc(string, total_str_mem_size);
    string[total_str_mem_size - 2] = ')';
    string[total_str_mem_size - 1] = '\0';
    return string;
}

/* ternary expressions */

static char const *ternary_to_string(Expr const *const expr) {
    assert(expr->type == ExprType_TERNARY);
    char const *str = "";
    switch (expr->value.ternary.type) {
    case ExprTernaryType_CONDITIONAL:
        str = "?:";
        break;
    }
    return ast_parenthesize(str, 3, expr->value.ternary.left,
                            expr->value.ternary.middle,
                            expr->value.ternary.right);
}

Expr *expr_init_ternary(Expr const *left, Token const *operator_left,
                        Expr const *middle, Token const *operator_right,
                        Expr const *right) {
    Expr *e = calloc(1, sizeof(Expr));
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

static char const *binary_to_string(Expr const *const expr) {
    assert(expr->type == ExprType_BINARY);
    char const *str = "";
    switch (expr->value.binary.type) {
    case ExprBinaryType_COMMA:
        str = ",";
        break;
    case ExprBinaryType_PLUS:
        str = "+";
        break;
    case ExprBinaryType_MINUS:
        str = "-";
        break;
    case ExprBinaryType_SLASH:
        str = "/";
        break;
    case ExprBinaryType_STAR:
        str = "*";
        break;
    case ExprBinaryType_GREATER:
        str = ">";
        break;
    case ExprBinaryType_GREATER_EQUAL:
        str = ">=";
        break;
    case ExprBinaryType_LESS_EQUAL:
        str = "<=";
        break;
    case ExprBinaryType_LESS:
        str = "<";
        break;
    case ExprBinaryType_BANG_EQUAL:
        str = "!=";
        break;
    case ExprBinaryType_EQUAL_EQUAL:
        str = "==";
        break;
    }
    return ast_parenthesize(str, 2, expr->value.binary.left,
                            expr->value.binary.right);
}
Expr *expr_init_binary(Expr const *left, Token const *const operator,
                       Expr const *right) {
    Expr *e = calloc(1, sizeof(Expr));
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

static char const *unary_to_string(Expr const *const expr) {
    assert(expr->type == ExprType_UNARY);
    char const *str = "";
    switch (expr->value.unary.type) {
    case ExprUnaryType_MINUS:
        str = "-";
        break;
    case ExprUnaryType_BANG:
        str = "!";
        break;
    }
    return ast_parenthesize(str, 1, expr->value.unary.right);
}
Expr *expr_init_unary(Token const *const operator, Expr const *const right) {
    Expr *e = calloc(1, sizeof(Expr));
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

/* literals */

/**
 * @brief Get the string representation of an expression literal.
 *
 * @param expr the expression literal.
 * @return A free-able pointer to the new string.
 */
static char const *literal_to_string(Expr const *const expr) {
    assert(expr->type == ExprType_LITERAL);
    char *str;
    size_t len;
    switch (expr->value.literal.type) {
    case ExprLiteralType_MISSING:
        str = calloc(1, 8);
        memcpy(str, "missing", 8);
        break;
    case ExprLiteralType_NIL:
        str = calloc(1, 4);
        memcpy(str, "nil", 4);
        break;
    case ExprLiteralType_TRUE:
        str = calloc(1, 5);
        memcpy(str, "true", 5);
        break;
    case ExprLiteralType_FALSE:
        str = calloc(1, 6);
        memcpy(str, "false", 6);
        break;
    case ExprLiteralType_IDENTIFIER:
    case ExprLiteralType_STRING:
        len = strlen(expr->value.literal.value) + 1;
        str = calloc(1, len);
        memcpy(str, expr->value.literal.value, len);
        break;
    case ExprLiteralType_NUMBER:
        str = malloc(50);
        snprintf(str, 50, "%f", *(double *)(expr->value.literal.value));
        break;
    }
    return str;
}

Expr *expr_init_literal(Token const *const token) {
    Expr *e = calloc(1, sizeof(Expr));
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

Expr *expr_init_missing(void) {
    Token *token = token_init(TokenType_MISSING, NULL, NULL, 0);
    Expr *literal = expr_init_literal(token);
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

static char const *grouping_to_string(Expr const *const expr) {
    assert(expr->type == ExprType_GROUPING);
    return ast_parenthesize("group", 1, expr->value.grouping.expression);
}
Expr *expr_init_grouping(Expr const *const expression) {
    Expr *e = calloc(1, sizeof(Expr));
    e->type = ExprType_GROUPING;
    e->value.grouping.expression = expression;
    return e;
}
