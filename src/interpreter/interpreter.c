#include "interpreter.h"
#include "../errorhandler.h"
#include "../parser/expr.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static bool had_error = false;

static void error(Expr const *const e, char const *const message) {
    errorhandler_printerror(e->line, message);
    had_error = true;
}

static Result const *evaluate(Expr const *const expr) {
    return interpreter_interpret(expr);
}

static bool is_truthy(Result const *const result) {
    if (result->type == ResultType_NULL) {
        return false;
    }

    if (result->type == ResultType_BOOLEAN) {
        return result->value.boolean;
    }

    return true;
}

static Result *equals(Result const *const first, Result const *const second) {
    Result *result = calloc(1, sizeof(Result));
    result->type = ResultType_BOOLEAN;
    if (first->type == ResultType_NUMBER && second->type == ResultType_NUMBER) {
        result->value.boolean = first->value.number == second->value.number;
    } else if (first->type == ResultType_STRING &&
               second->type == ResultType_STRING) {
        result->value.boolean =
            strcmp(first->value.string, second->value.string) != 0;
    } else if (first->type == ResultType_BOOLEAN &&
               second->type == ResultType_BOOLEAN) {
        result->value.boolean = first->value.boolean == second->value.boolean;
    } else {
        result->value.boolean = false;
    }
    return result;
}

static Result const *interpret_literal(Expr const *const expr) {
    ExprLiteral const literal = expr->value.literal;
    Result *result = calloc(1, sizeof(Result));

    switch (literal.type) {
    case ExprLiteralType_MISSING:
        error(expr, "Missing value.");
        free(result);
        return NULL;
    case ExprLiteralType_NIL:
        result->type = ResultType_NULL;
        break;
    case ExprLiteralType_TRUE:
        result->type = ResultType_BOOLEAN;
        result->value.boolean = true;
        break;
    case ExprLiteralType_FALSE:
        result->type = ResultType_BOOLEAN;
        result->value.boolean = false;
        break;
    case ExprLiteralType_IDENTIFIER:
        result->type = ResultType_IDENTIFIER;
        break;
    case ExprLiteralType_STRING:
        result->type = ResultType_STRING;
        result->value.string = literal.value;
        break;
    case ExprLiteralType_NUMBER:
        result->type = ResultType_NUMBER;
        result->value.number = *(double *)literal.value;
        break;
    }
    return result;
}

static Result const *interpret_grouping(Expr const *const expr) {
    return evaluate(expr->value.grouping.expression);
}

static Result const *interpret_unary(Expr const *const expr) {
    ExprUnary const unary = expr->value.unary;
    Result const *right_result = evaluate(unary.right);
    if (had_error) {
        return NULL;
    }
    Result *result = calloc(1, sizeof(Result));

    switch (unary.type) {
    case ExprUnaryType_MINUS:
        if (right_result->type != ResultType_NUMBER) {
            error(expr, "Operand must be a number.");
            free(result);
            return NULL;
        }
        result->type = ResultType_NUMBER;
        result->value.number = -1 * right_result->value.number;
        break;
    case ExprUnaryType_BANG:
        result->type = ResultType_BOOLEAN;
        result->value.boolean = is_truthy(right_result);
        break;
    }

    return result;
}

static Result const *interpret_binary(Expr const *const expr) {
    Result const *l = evaluate(expr->value.binary.left);
    Result const *r = evaluate(expr->value.binary.right);
    Result *result = calloc(1, sizeof(Result));

    switch (expr->value.binary.type) {
    case ExprBinaryType_MINUS:
        if (l->type != ResultType_NUMBER || r->type != ResultType_NUMBER) {
            error(expr, "Both operands must be numbers.");
            free(result);
            return NULL;
        }
        result->type = ResultType_NUMBER;
        result->value.number = l->value.number - r->value.number;
        break;
    case ExprBinaryType_PLUS:
        if (l->type == ResultType_NUMBER || r->type == ResultType_NUMBER) {
            result->type = ResultType_NUMBER;
            result->value.number = l->value.number + r->value.number;
        } else if (l->type == ResultType_STRING &&
                   r->type == ResultType_STRING) {
            result->type = ResultType_STRING;
            char *str =
                calloc(strlen(l->value.string) + strlen(r->value.string) + 1,
                       sizeof(char));
            strcat(str, l->value.string);
            strcat(str + strlen(l->value.string), r->value.string);
            result->value.string = str;
        } else {
            error(expr, "Both operands must be numbers or both operands must "
                        "be strings.");
            free(result);
            return NULL;
        }
        break;
    case ExprBinaryType_SLASH:
        if (l->type != ResultType_NUMBER || r->type != ResultType_NUMBER) {
            error(expr, "Both operands must be numbers.");
            free(result);
            return NULL;
        }
        result->type = ResultType_NUMBER;
        result->value.number = l->value.number / r->value.number;
        break;
    case ExprBinaryType_STAR:
        if (l->type != ResultType_NUMBER || r->type != ResultType_NUMBER) {
            error(expr, "Both operands must be numbers.");
            free(result);
            return NULL;
        }
        result->type = ResultType_NUMBER;
        result->value.number = l->value.number * r->value.number;
        break;
    case ExprBinaryType_GREATER:
        if (l->type != ResultType_NUMBER || r->type != ResultType_NUMBER) {
            error(expr, "Both operands must be numbers.");
            free(result);
            return NULL;
        }
        result->type = ResultType_BOOLEAN;
        result->value.boolean = l->value.number > r->value.number;
        break;
    case ExprBinaryType_GREATER_EQUAL:
        if (l->type != ResultType_NUMBER || r->type != ResultType_NUMBER) {
            error(expr, "Both operands must be numbers.");
            free(result);
            return NULL;
        }
        result->type = ResultType_BOOLEAN;
        result->value.boolean = l->value.number >= r->value.number;
        break;
    case ExprBinaryType_LESS_EQUAL:
        if (l->type != ResultType_NUMBER || r->type != ResultType_NUMBER) {
            error(expr, "Both operands must be numbers.");
            free(result);
            return NULL;
        }
        result->type = ResultType_BOOLEAN;
        result->value.boolean = l->value.number <= r->value.number;
        break;
    case ExprBinaryType_LESS:
        if (l->type != ResultType_NUMBER || r->type != ResultType_NUMBER) {
            error(expr, "Both operands must be numbers.");
            free(result);
            return NULL;
        }
        result->type = ResultType_BOOLEAN;
        result->value.boolean = l->value.number < r->value.number;
        break;
    case ExprBinaryType_BANG_EQUAL:
        free(result);
        result = equals(l, r);
        result->value.boolean = !result->value.boolean;
        break;
    case ExprBinaryType_EQUAL_EQUAL:
        free(result);
        result = equals(l, r);
        break;
    }
    return result;
}

static Result const *interpret_ternary(Expr const *const expr) {
    Result const *l = evaluate(expr->value.ternary.left);
    switch (expr->value.ternary.type) {
    case ExprTernaryType_CONDITIONAL:
        if (is_truthy(l)) {
            return evaluate(expr->value.ternary.middle);
        } else {
            return evaluate(expr->value.ternary.right);
        }
        break;
    }
    return NULL;
}

Result const *interpreter_interpret(Expr const *const expr) {
    switch (expr->type) {
    case ExprType_UNARY:
        return interpret_unary(expr);
    case ExprType_BINARY:
        return interpret_binary(expr);
    case ExprType_TERNARY:
        return interpret_ternary(expr);
    case ExprType_LITERAL:
        return interpret_literal(expr);
    case ExprType_GROUPING:
        return interpret_grouping(expr);
    }
    return NULL;
}
