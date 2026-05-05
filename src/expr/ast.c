#include "ast.h"
#include "expr.h"
#include <stdlib.h>

char const *ast_print(Expr *expr, ExprVisitor *visitor) {
    return expr->accept(expr, visitor);
}

static void *visit_binary(ExprBinary *expr) {
    return (void *)expr->super.to_string(expr);
}
static void *visit_unary(ExprUnary *expr) {
    return (void *)expr->super.to_string(expr);
}
static void *visit_literal(ExprLiteral *expr) {
    return (void *)expr->super.to_string(expr);
}
static void *visit_grouping(ExprGrouping *expr) {
    return (void *)expr->super.to_string(expr);
}
ExprVisitor *ast_init_printer(void) {
    ExprVisitor *printer = calloc(1, sizeof(ExprVisitor));
    printer->visit_binary = visit_binary;
    printer->visit_unary = visit_unary;
    printer->visit_literal = visit_literal;
    printer->visit_grouping = visit_grouping;
    return printer;
}
