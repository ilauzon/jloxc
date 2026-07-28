/*
 * Defines function expr_to_string in exprstr.h, which generates a string
 * representation of an Expr.
 *
 * This exists only for debugging purposes. It is not to be used in the main
 * control flow of the program.
 */

#include "expr.h"
#include <assert.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

static char const *var_to_string(Expr const *const expr);
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
    case ExprType_VAR:
        return var_to_string(expr);
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

static char const *var_to_string(Expr const *const expr) {
    assert(expr->type == ExprType_VAR);
    size_t len = strlen(expr->value.var.name) + 1;
    char *str = calloc(len, sizeof(char));
    memcpy(str, expr->value.var.name, len);
    return str;
}

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
        break;
    }
    return str;
}

static char const *grouping_to_string(Expr const *const expr) {
    assert(expr->type == ExprType_GROUPING);
    return ast_parenthesize("group", 1, expr->value.grouping.expression);
}
