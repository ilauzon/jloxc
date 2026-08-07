#pragma once
#include "../parser/expr.h"
#include "environment.h"
#include <stddef.h>

typedef struct {
    Environment *state;
    ArenaAllocator *allocator;
    bool had_error;
} Interpreter;

Interpreter *interpreter_init(void);
void interpreter_destroy(Interpreter *interpreter);
void interpreter_interpret(Interpreter *interpreter, size_t const stmt_count,
                           Stmt *stmts[stmt_count]);
