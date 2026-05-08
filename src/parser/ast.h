#pragma once
#include "expr.h"

char const *ast_print(Expr *expr, ExprVisitor *visitor);

ExprVisitor *ast_init_printer(void);
