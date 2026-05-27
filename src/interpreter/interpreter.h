#pragma once
#include "../parser/expr.h"
#include <stddef.h>

enum ResultType {
    ResultType_NULL,
    ResultType_STRING,
    ResultType_NUMBER,
    ResultType_BOOLEAN,
    ResultType_IDENTIFIER,
};

/** The result of interpreting an `Expr`. */
typedef struct {
    // the type-tag of the result.
    enum ResultType type;
    union {
        char const *string;
        double number;
        bool boolean;
    } value;
} Result;

typedef struct {
    void *state;
} Interpreter;

Interpreter *interpreter_init(void);
void interpreter_interpret(Interpreter *interpreter, size_t const stmt_count,
                           Stmt const stmts[stmt_count]);
