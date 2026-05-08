#include "expr.h"
#include <stdarg.h>
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

static void *binary_accept(void const *const visitee,
                           ExprVisitor const *const visitor) {
    return visitor->visit_binary(visitee);
}
static char const *binary_to_string(void const *const expr) {
    ExprBinary *e = (ExprBinary *)expr;
    return ast_parenthesize(e->operator->lexeme, 2, e->left, e->right);
}
ExprBinary *expr_init_binary(Expr const *left, Token const *const operator,
                             Expr const *right) {
    ExprBinary *e = calloc(1, sizeof(ExprBinary));
    e->super.accept = binary_accept;
    e->super.to_string = binary_to_string;
    e->left = left;
    e->operator = operator;
    e->right = right;
    return e;
}

/* unary expressions */

static void *unary_accept(void const *const visitee,
                          ExprVisitor const *const visitor) {
    return visitor->visit_unary(visitee);
}
static char const *unary_to_string(void const *const expr) {
    ExprUnary *e = (ExprUnary *)expr;
    return ast_parenthesize(e->operator->lexeme, 1, e->right);
}
ExprUnary *expr_init_unary(Token const *const operator,
                           Expr const *const right) {
    ExprUnary *e = calloc(1, sizeof(ExprUnary));
    e->super.accept = unary_accept;
    e->super.to_string = unary_to_string;
    e->operator = operator;
    e->right = right;
    return e;
}

/* literals */

static void *literal_accept(void const *const visitee,
                            ExprVisitor const *const visitor) {
    return visitor->visit_literal(visitee);
}
static char const *literal_to_string(void const *const expr) {
    ExprLiteral *e = (ExprLiteral *)expr;
    if (e->literal->value == NULL) {
        char *str = calloc(1, 4);
        memcpy(str, "nil", 4);
    }
    return (e->literal->to_string)(e->literal);
}
ExprLiteral *expr_init_literal(Literal const *const literal) {
    ExprLiteral *e = calloc(1, sizeof(ExprLiteral));
    e->super.accept = literal_accept;
    e->super.to_string = literal_to_string;
    e->literal = literal;
    return e;
}

/* groupings */

static void *grouping_accept(void const *const visitee,
                             ExprVisitor const *const visitor) {
    return visitor->visit_grouping(visitee);
}
static char const *grouping_to_string(void const *const expr) {
    ExprGrouping *e = (ExprGrouping *)expr;
    return ast_parenthesize("group", 1, e->expression);
}
ExprGrouping *expr_init_grouping(Expr const *const expression) {
    ExprGrouping *e = calloc(1, sizeof(ExprGrouping));
    e->super.accept = grouping_accept;
    e->super.to_string = grouping_to_string;
    e->expression = expression;
    return e;
}
