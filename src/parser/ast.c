#include "ast.h"
#include "expr.h"
#include <stdlib.h>

char const *ast_print(Expr *expr, ExprVisitor *visitor) {
    return expr->accept(expr, visitor);
}

static void *visit_binary(ExprBinary const *expr) {
    return (void *)expr->super.to_string(expr);
}
static void *visit_unary(ExprUnary const *expr) {
    return (void *)expr->super.to_string(expr);
}
static void *visit_literal(ExprLiteral const *expr) {
    return (void *)expr->super.to_string(expr);
}
static void *visit_grouping(ExprGrouping const *expr) {
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
