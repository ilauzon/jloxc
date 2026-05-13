#include "expr.h"
#include "../errorhandler.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
        char const *expr_str = expr->to_string(expr);
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

/* binary expressions */

static char const *binary_to_string(void const *const expr) {
    ExprBinary *e = (ExprBinary *)expr;
    return ast_parenthesize(e->operator->lexeme, 2, e->left, e->right);
}
ExprBinary *expr_init_binary(Expr const *left, Token const *const operator,
                             Expr const *right) {
    ExprBinary *e = calloc(1, sizeof(ExprBinary));
    e->super.to_string = binary_to_string;
    e->left = left;
    e->operator = operator;
    e->right = right;
    return e;
}

/* unary expressions */

static char const *unary_to_string(void const *const expr) {
    ExprUnary *e = (ExprUnary *)expr;
    return ast_parenthesize(e->operator->lexeme, 1, e->right);
}
ExprUnary *expr_init_unary(Token const *const operator,
                           Expr const *const right) {
    ExprUnary *e = calloc(1, sizeof(ExprUnary));
    e->super.to_string = unary_to_string;
    e->operator = operator;
    e->right = right;
    return e;
}

/* literals */

/**
 * @brief Get the string representation of an expression literal.
 *
 * @param expr the expression literal.
 * @return A free-able pointer to the new string.
 */
static char const *literal_to_string(void const *const expr) {
    ExprLiteral *e = (ExprLiteral *)expr;
    char *str;
    size_t len;
    switch (e->type) {
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
        len = strlen(e->value) + 1;
        str = calloc(1, len);
        memcpy(str, e->value, len);
        break;
    case TokenType_NUMBER:
        str = malloc(50);
        snprintf(str, 50, "%f", *(double *)(e->value));
        break;
    default:
        str = calloc(1, 1); // free-able empty string in default case
    }
    return str;
}

bool expr_type_is_literal(enum TokenType const type) {
    switch (type) {
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

ExprLiteral *expr_init_literal(Token const *const token) {
    if (!expr_type_is_literal(token->type)) {
        errorhandler_printerror_token(
            token, "Attempted call to instantiate literal token as ExprLiteral "
                   "with incompatible TokenType, stopping!");
        return NULL;
    }
    ExprLiteral *e = calloc(1, sizeof(ExprLiteral));
    e->super.to_string = literal_to_string;
    e->type = token->type;

    switch (token->type) {
    case TokenType_IDENTIFIER:
        e->value = token->lexeme;
        break;
    case TokenType_STRING:
        e->value = token->literal->value;
        break;
    case TokenType_NUMBER:
        e->value = token->literal->value;
        break;
    default:
        e->value = 0;
    }

    return e;
}

/* groupings */

static char const *grouping_to_string(void const *const expr) {
    ExprGrouping *e = (ExprGrouping *)expr;
    return ast_parenthesize("group", 1, e->expression);
}
ExprGrouping *expr_init_grouping(Expr const *const expression) {
    ExprGrouping *e = calloc(1, sizeof(ExprGrouping));
    e->super.to_string = grouping_to_string;
    e->expression = expression;
    return e;
}
