#include "interpreter.h"
#include "../errors/errorhandler.h"
#include "../parser/expr.h"
#include "result.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static bool had_error = false;

static Result const *evaluate(Interpreter const interpreter,
                              Expr const *const expr);

static Result const *read_defined_variable(Interpreter const interpreter,
                                           Result const *const variable) {
    if (variable->type != ResultType_IDENTIFIER) {
        errorhandler_printerror(variable->line,
                                "Internal error, non-identifier Result passed "
                                "into read_defined_variable.");
        return NULL;
    }
    Result const *value =
        environment_read(*interpreter.state, variable->value.name);
    if (value == NULL) {
        char message[100] = {0};
        snprintf(message, 99, "Variable '%s' is not defined.",
                 variable->value.name);
        errorhandler_printerror(variable->line, message);
        had_error = true;
    }
    return value;
}

static void print_result(Interpreter const interpreter,
                         Result const *const result) {
    switch (result->type) {
    case ResultType_NULL:
        printf("<nil>\n");
        break;
    case ResultType_STRING:
        printf("%s\n", result->value.string);
        break;
    case ResultType_NUMBER:
        printf("%f\n", result->value.number);
        break;
    case ResultType_BOOLEAN:
        if (result->value.boolean) {
            printf("<true>\n");
        } else {
            printf("<false>\n");
        }
        break;
    case ResultType_IDENTIFIER:
        Result const *value = read_defined_variable(interpreter, result);
        if (had_error) {
            return;
        }
        if (value == NULL) {
            printf("<uninitialized>\n");
        } else {
            print_result(interpreter, value);
        }
        break;
    }
}

static void error(Expr const *const e, char const *const message) {
    errorhandler_printerror(e->line, message);
    had_error = true;
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

static Result *equals(Interpreter const interpreter, Result const *const first,
                      Result const *const second) {
    Result *result = arena_allocate(interpreter.allocator, sizeof(Result));
    result->line = first->line;
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

static Result const *interpret_literal(Interpreter const interpreter,
                                       Expr const *const expr) {
    ExprLiteral const literal = expr->value.literal;
    Result *result = arena_allocate(interpreter.allocator, sizeof(Result));
    result->line = expr->line;

    switch (literal.type) {
    case ExprLiteralType_MISSING:
        error(expr, "Missing value.");
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
        result->value.name = arena_strdup(interpreter.allocator, literal.value);
        break;
    case ExprLiteralType_STRING:
        result->type = ResultType_STRING;
        result->value.string =
            arena_strdup(interpreter.allocator, literal.value);
        break;
    case ExprLiteralType_NUMBER:
        result->type = ResultType_NUMBER;
        result->value.number = *(double *)literal.value;
        break;
    }
    return result;
}

static Result const *interpret_var(Interpreter const interpreter,
                                   Expr const *const expr) {
    return environment_read(*interpreter.state, expr->value.var.name);
}

static Result const *interpret_grouping(Interpreter const interpreter,
                                        Expr const *const expr) {
    return evaluate(interpreter, expr->value.grouping.expression);
}

static Result const *interpret_unary(Interpreter const interpreter,
                                     Expr const *const expr) {
    ExprUnary const unary = expr->value.unary;
    Result const *right_result = evaluate(interpreter, unary.right);
    if (had_error) {
        return NULL;
    }
    if (right_result->type == ResultType_IDENTIFIER) {
        right_result = read_defined_variable(interpreter, right_result);
    }
    if (had_error) {
        return NULL;
    }
    Result *result = arena_allocate(interpreter.allocator, sizeof(Result));
    result->line = expr->line;

    switch (unary.type) {
    case ExprUnaryType_MINUS:
        if (right_result->type != ResultType_NUMBER) {
            error(expr, "Operand must be a number.");
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

static Result const *interpret_binary(Interpreter const interpreter,
                                      Expr const *const expr) {
    Result const *l = evaluate(interpreter, expr->value.binary.left);
    if (had_error) {
        return NULL;
    }
    if (l->type == ResultType_IDENTIFIER) {
        Result const *result = read_defined_variable(interpreter, l);
        l = result;
    }

    Result const *r = evaluate(interpreter, expr->value.binary.right);
    if (had_error) {
        return NULL;
    }
    if (r->type == ResultType_IDENTIFIER) {
        Result const *result = read_defined_variable(interpreter, r);
        r = result;
    }

    if (had_error) {
        return NULL;
    }

    Result *result = arena_allocate(interpreter.allocator, sizeof(Result));
    result->line = expr->line;

    Result const *ret = NULL;

    switch (expr->value.binary.type) {
    case ExprBinaryType_COMMA:
        ret = r;
        break;
    case ExprBinaryType_MINUS:
        if (l->type != ResultType_NUMBER || r->type != ResultType_NUMBER) {
            error(expr, "Both operands must be numbers.");
            break;
        }
        result->type = ResultType_NUMBER;
        result->value.number = l->value.number - r->value.number;
        ret = result;
        break;
    case ExprBinaryType_PLUS:
        if (l->type == ResultType_NUMBER && r->type == ResultType_NUMBER) {
            result->type = ResultType_NUMBER;
            result->value.number = l->value.number + r->value.number;
            ret = result;
        } else if (l->type == ResultType_STRING &&
                   r->type == ResultType_STRING) {
            result->type = ResultType_STRING;
            char *str =
                arena_allocate(interpreter.allocator,
                               strlen(l->value.string) +
                                   strlen(r->value.string) + 1 * sizeof(char));
            strcat(str, l->value.string);
            strcat(str + strlen(l->value.string), r->value.string);
            result->value.string = str;
            ret = result;
        } else {
            error(expr, "Both operands must be numbers or both operands must "
                        "be strings.");
        }
        break;
    case ExprBinaryType_SLASH:
        if (l->type != ResultType_NUMBER || r->type != ResultType_NUMBER) {
            error(expr, "Both operands must be numbers.");
        }
        result->type = ResultType_NUMBER;
        result->value.number = l->value.number / r->value.number;
        ret = result;
        break;
    case ExprBinaryType_STAR:
        if (l->type != ResultType_NUMBER || r->type != ResultType_NUMBER) {
            error(expr, "Both operands must be numbers.");
        }
        result->type = ResultType_NUMBER;
        result->value.number = l->value.number * r->value.number;
        ret = result;
        break;
    case ExprBinaryType_GREATER:
        if (l->type != ResultType_NUMBER || r->type != ResultType_NUMBER) {
            error(expr, "Both operands must be numbers.");
        }
        result->type = ResultType_BOOLEAN;
        result->value.boolean = l->value.number > r->value.number;
        ret = result;
        break;
    case ExprBinaryType_GREATER_EQUAL:
        if (l->type != ResultType_NUMBER || r->type != ResultType_NUMBER) {
            error(expr, "Both operands must be numbers.");
        }
        result->type = ResultType_BOOLEAN;
        result->value.boolean = l->value.number >= r->value.number;
        ret = result;
        break;
    case ExprBinaryType_LESS_EQUAL:
        if (l->type != ResultType_NUMBER || r->type != ResultType_NUMBER) {
            error(expr, "Both operands must be numbers.");
        }
        result->type = ResultType_BOOLEAN;
        result->value.boolean = l->value.number <= r->value.number;
        ret = result;
        break;
    case ExprBinaryType_LESS:
        if (l->type != ResultType_NUMBER || r->type != ResultType_NUMBER) {
            error(expr, "Both operands must be numbers.");
        }
        result->type = ResultType_BOOLEAN;
        result->value.boolean = l->value.number < r->value.number;
        ret = result;
        break;
    case ExprBinaryType_BANG_EQUAL:
        result = equals(interpreter, l, r);
        result->value.boolean = !result->value.boolean;
        ret = result;
        break;
    case ExprBinaryType_EQUAL_EQUAL:
        result = equals(interpreter, l, r);
        ret = result;
        break;
    }

    return ret;
}

static Result const *interpret_ternary(Interpreter const interpreter,
                                       Expr const *const expr) {
    Result const *l = evaluate(interpreter, expr->value.ternary.left);
    if (had_error) {
        return NULL;
    }
    if (l->type == ResultType_IDENTIFIER) {
        Result const *value = read_defined_variable(interpreter, l);
        l = value;
    }
    if (had_error) {
        return NULL;
    }
    Result const *ret = NULL;
    switch (expr->value.ternary.type) {
    case ExprTernaryType_CONDITIONAL:
        if (is_truthy(l)) {
            ret = evaluate(interpreter, expr->value.ternary.middle);
            if (had_error) {
                return NULL;
            }
        } else {
            ret = evaluate(interpreter, expr->value.ternary.right);
            if (had_error) {
                return NULL;
            }
        }
        break;
    }
    if (ret->type == ResultType_IDENTIFIER) {
        ret = read_defined_variable(interpreter, ret);
    }
    return ret;
}

static Result const *evaluate(Interpreter const interpreter,
                              Expr const *const expr) {
    switch (expr->type) {
    case ExprType_VAR:
        return interpret_var(interpreter, expr);
    case ExprType_UNARY:
        return interpret_unary(interpreter, expr);
    case ExprType_BINARY:
        return interpret_binary(interpreter, expr);
    case ExprType_TERNARY:
        return interpret_ternary(interpreter, expr);
    case ExprType_LITERAL:
        return interpret_literal(interpreter, expr);
    case ExprType_GROUPING:
        return interpret_grouping(interpreter, expr);
    }
    return NULL;
}

static void interpret_stmt_expr(Interpreter const interpreter,
                                Stmt const *const stmt) {
    Result const *const value =
        evaluate(interpreter, stmt->value.print.expression);
    if (had_error) {
        return;
    }
}

static void interpret_stmt_print(Interpreter const interpreter,
                                 Stmt const *const stmt) {
    Result const *const value =
        evaluate(interpreter, stmt->value.print.expression);
    if (had_error) {
        return;
    }
    print_result(interpreter, value);
}

static void interpret_stmt_var(Interpreter const interpreter, Stmt const stmt) {
    char const *const key = stmt.value.var.name;
    Expr const *const initializer = stmt.value.var.initializer;
    Result value;
    if (initializer != NULL) {
        value = *evaluate(interpreter, stmt.value.var.initializer);
        if (had_error) {
            return;
        }
        if (value.type == ResultType_IDENTIFIER) {
            value = *read_defined_variable(interpreter, &value);
            if (had_error) {
                return;
            }
        }
    } else {
    }
    environment_define(interpreter.state,
                       (EnvironmentVariable){.key = key, .value = &value});
}

Interpreter *interpreter_init(void) {
    Interpreter *interpreter = calloc(1, sizeof(Interpreter));
    Environment *env = environment_init(100);
    interpreter->state = env;
    ArenaAllocator *allocator = arena_init();
    interpreter->allocator = allocator;
    return interpreter;
}

void interpreter_destroy(Interpreter *interpreter) {
    arena_destroy(interpreter->allocator);
    environment_destroy(interpreter->state);
    free(interpreter);
}

void interpreter_interpret(Interpreter const interpreter,
                           size_t const stmt_count, Stmt *stmts[stmt_count]) {
    for (size_t i = 0; i < stmt_count; ++i) {

        // clear all memory not in the environment before each statement
        arena_destroy_until_mark(interpreter.allocator);

        Stmt const *stmt = stmts[i];
        switch (stmt->type) {
        case StmtType_EXPR:
            interpret_stmt_expr(interpreter, stmt);
            break;
        case StmtType_PRINT:
            interpret_stmt_print(interpreter, stmt);
            break;
        case StmtType_VAR:
            interpret_stmt_var(interpreter, *stmt);
            break;
        }

        if (had_error) {
            had_error = false;
            break;
        }
    }
}
