#pragma once
#include <stdbool.h>

enum ResultType {
    ResultType_NULL,
    ResultType_STRING,
    ResultType_NUMBER,
    ResultType_BOOLEAN,
};

/** The result of interpreting an `Expr`. */
typedef struct {
    // the type-tag of the result.
    enum ResultType type;
    int line;
    union {
        char const *string;
        char const *name;
        double number;
        bool boolean;
    } value;
} Result;
