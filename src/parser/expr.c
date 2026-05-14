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
    default:
        assert(false);
        return NULL;
    }
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
    size_t op1_strlen = strlen(expr->value.ternary.operator_left->lexeme);
    size_t op2_strlen = strlen(expr->value.ternary.operator_right->lexeme);
    char *str = calloc(1, op1_strlen + op2_strlen + 1);
    memcpy(str, expr->value.ternary.operator_left->lexeme, op1_strlen);
    memcpy(str + op1_strlen, expr->value.ternary.operator_right->lexeme,
           op2_strlen);
    return ast_parenthesize(str, 3, expr->value.ternary.left,
                            expr->value.ternary.middle,
                            expr->value.ternary.right);
    free(str);
}

Expr *expr_init_ternary(Expr const *left, Token const *operator_left,
                        Expr const *middle, Token const *operator_right,
                        Expr const *right) {
    Expr *e = calloc(1, sizeof(Expr));
    e->type = ExprType_TERNARY;
    e->value.ternary.left = left;
    e->value.ternary.operator_left = operator_left;
    e->value.ternary.middle = middle;
    e->value.ternary.operator_right = operator_right;
    e->value.ternary.right = right;
    return e;
}

/* binary expressions */

static char const *binary_to_string(Expr const *const expr) {
    return ast_parenthesize(expr->value.binary.operator->lexeme, 2,
                            expr->value.binary.left, expr->value.binary.right);
}
Expr *expr_init_binary(Expr const *left, Token const *const operator,
                       Expr const *right) {
    Expr *e = calloc(1, sizeof(Expr));
    e->type = ExprType_BINARY;
    e->value.binary.left = left;
    e->value.binary.operator = operator;
    e->value.binary.right = right;
    return e;
}

/* unary expressions */

static char const *unary_to_string(Expr const *const expr) {
    return ast_parenthesize(expr->value.unary.operator->lexeme, 1,
                            expr->value.unary.right);
}
Expr *expr_init_unary(Token const *const operator, Expr const *const right) {
    Expr *e = calloc(1, sizeof(Expr));
    e->type = ExprType_UNARY;
    e->value.unary.operator = operator;
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
    case TokenType_MISSING:
        str = calloc(1, 8);
        memcpy(str, "missing", 8);
        break;
    case TokenType_NIL:
        str = calloc(1, 4);
        memcpy(str, "nil", 4);
        break;
    case TokenType_TRUE:
        str = calloc(1, 5);
        memcpy(str, "true", 5);
        break;
    case TokenType_FALSE:
        str = calloc(1, 5);
        memcpy(str, "false", 5);
        break;
    case TokenType_IDENTIFIER:
    case TokenType_STRING:
        len = strlen(expr->value.literal.value) + 1;
        str = calloc(1, len);
        memcpy(str, expr->value.literal.value, len);
        break;
    case TokenType_NUMBER:
        str = malloc(50);
        snprintf(str, 50, "%f", *(double *)(expr->value.literal.value));
        break;
    default:
        str = calloc(1, 1); // free-able empty string in default case
    }
    return str;
}

bool expr_type_is_literal(enum TokenType const type) {
    switch (type) {
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

Expr *expr_init_literal(Token const *const token) {
    if (!expr_type_is_literal(token->type)) {
        errorhandler_printerror_token(
            token, "Attempted call to instantiate literal token as ExprLiteral "
                   "with incompatible TokenType, stopping!");
        return NULL;
    }
    Expr *e = calloc(1, sizeof(Expr));
    e->type = ExprType_LITERAL;
    e->value.literal.type = token->type;

    switch (token->type) {
    case TokenType_IDENTIFIER:
        e->value.literal.value = token->lexeme;
        break;
    case TokenType_STRING:
        e->value.literal.value = token->literal->value;
        break;
    case TokenType_NUMBER:
        e->value.literal.value = token->literal->value;
        break;
    default:
        e->value.literal.value = 0;
    }

    return e;
}

Expr *expr_init_missing(void) {
    Token *token = token_init(TokenType_MISSING, NULL, NULL, 0);
    Expr *literal = expr_init_literal(token);
    free(token);
    return literal;
}

/* groupings */

static char const *grouping_to_string(Expr const *const expr) {
    return ast_parenthesize("group", 1, expr->value.grouping.expression);
}
Expr *expr_init_grouping(Expr const *const expression) {
    Expr *e = calloc(1, sizeof(Expr));
    e->type = ExprType_GROUPING;
    e->value.grouping.expression = expression;
    return e;
}
