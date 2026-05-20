#pragma once
#include "../parser/expr.h"

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

Result const *interpreter_interpret(Expr const *const expr);
