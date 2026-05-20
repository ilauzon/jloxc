#include "interpreter.h"
#include "../parser/expr.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

Result const *evaluate(Expr const *const expr) {
    return interpreter_interpret(expr);
}

bool is_truthy(Result const *const result) {
    if (result->type == ResultType_NULL) {
        return false;
    }

    if (result->type == ResultType_BOOLEAN) {
        return result->value.boolean;
    }

    return true;
}

Result *equals(Result const *const first, Result const *const second) {
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
        // TODO emit error
        free(result);
        return NULL;
    }
    return result;
}

Result const *interpret_literal(Expr const *const expr) {
    ExprLiteral const literal = expr->value.literal;
    Result *result = calloc(1, sizeof(Result));

    switch (literal.type) {
    case ExprLiteralType_MISSING:
        // TODO emit error
        result->type = ResultType_NULL;
        break;
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

Result const *interpret_grouping(Expr const *const expr) {
    return evaluate(expr->value.grouping.expression);
}

Result const *interpret_unary(Expr const *const expr) {
    ExprUnary const unary = expr->value.unary;
    Result const *right_result = evaluate(unary.right);
    Result *result = calloc(1, sizeof(Result));

    switch (unary.type) {
    case ExprUnaryType_MINUS:
        if (right_result->type != ResultType_NUMBER) {
            // TODO emit error
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

Result const *interpret_binary(Expr const *const expr) {
    Result const *l = evaluate(expr->value.binary.left);
    Result const *r = evaluate(expr->value.binary.right);
    Result *result = calloc(1, sizeof(Result));

    switch (expr->value.binary.type) {
    case ExprBinaryType_MINUS:
        if (l->type != ResultType_NUMBER || r->type != ResultType_NUMBER) {
            // TODO emit error
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
            // TODO emit error
        }
        break;
    case ExprBinaryType_SLASH:
        if (l->type != ResultType_NUMBER || r->type != ResultType_NUMBER) {
            // TODO emit error
        }
        result->type = ResultType_NUMBER;
        result->value.number = l->value.number / r->value.number;
        break;
    case ExprBinaryType_STAR:
        if (l->type != ResultType_NUMBER || r->type != ResultType_NUMBER) {
            // TODO emit error
        }
        result->type = ResultType_NUMBER;
        result->value.number = l->value.number * r->value.number;
        break;
    case ExprBinaryType_GREATER:
        if (l->type != ResultType_NUMBER || r->type != ResultType_NUMBER) {
            // TODO emit error
        }
        result->type = ResultType_BOOLEAN;
        result->value.boolean = l->value.number > r->value.number;
        break;
    case ExprBinaryType_GREATER_EQUAL:
        if (l->type != ResultType_NUMBER || r->type != ResultType_NUMBER) {
            // TODO emit error
        }
        result->type = ResultType_BOOLEAN;
        result->value.boolean = l->value.number >= r->value.number;
        break;
    case ExprBinaryType_LESS_EQUAL:
        if (l->type != ResultType_NUMBER || r->type != ResultType_NUMBER) {
            // TODO emit error
        }
        result->type = ResultType_BOOLEAN;
        result->value.boolean = l->value.number <= r->value.number;
        break;
    case ExprBinaryType_LESS:
        if (l->type != ResultType_NUMBER || r->type != ResultType_NUMBER) {
            // TODO emit error
        }
        result->type = ResultType_BOOLEAN;
        result->value.boolean = l->value.number < r->value.number;
        break;
    case ExprBinaryType_BANG_EQUAL:
        free(result);
        result = equals(l, r);
        // TODO emit error if equals was problematic
        result->value.boolean = !result->value.boolean;
        break;
    case ExprBinaryType_EQUAL_EQUAL:
        free(result);
        result = equals(l, r);
        // TODO emit error if equals was problematic
        break;
    }
    return result;
}

Result const *interpreter_interpret(Expr const *const expr) {
    switch (expr->type) {
    case ExprType_UNARY:
        return interpret_unary(expr);
    case ExprType_BINARY:
        return interpret_binary(expr);
    case ExprType_TERNARY:
    case ExprType_LITERAL:
        return interpret_literal(expr);
    case ExprType_GROUPING:
        return interpret_grouping(expr);
    }
    return NULL;
}
