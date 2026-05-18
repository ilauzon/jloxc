#include "interpreter.h"
#include "../parser/expr.h"
#include <stdlib.h>
#include <string.h>

void const *evaluate(Expr const *const expr) {
    return interpreter_interpret(expr);
}

void const *interpret_literal(Expr const *const expr) {
    return expr->value.literal.value;
}

void const *interpret_grouping(Expr const *const expr) {
    return evaluate(expr->value.grouping.expression);
}

void const *interpret_unary(Expr const *const expr) {
    void const *right = evaluate(expr->value.unary.right);

    switch (expr->value.unary.type) {
    case ExprUnaryType_MINUS:
        // assume double
        ;
        double *negated = calloc(1, sizeof(double));
        memcpy(negated, right, sizeof(double));
        *negated *= -1;
        return negated;
    case ExprUnaryType_BANG:
        break;
    }

    return NULL;
}

void const *interpreter_interpret(Expr const *const expr) {
    switch (expr->type) {
    case ExprType_UNARY:
    case ExprType_BINARY:
    case ExprType_TERNARY:
    case ExprType_LITERAL:
        return interpret_literal(expr);
    case ExprType_GROUPING:
        return interpret_grouping(expr);
    }
}
