#include "errorhandler.h"
#include "token.h"
#include "tokentype.h"
#include <stdio.h>

static bool had_error = false;

static void report(int line, char const where[static 1],
                   char const message[static 1]) {
    had_error = true;
    printf("[line %d] Error %s: %s\n", line, where, message);
}

bool errorhandler_haderror(void) { return had_error; }

void errorhandler_reseterrors(void) { had_error = false; }

void errorhandler_printerror(int line, char const message[static 1]) {
    report(line, "", message);
}

void errorhandler_printerror_token(Token const *const token,
                                   char const message[static 1]) {
    if (token->type == TokenType_EOF) {
        report(token->line, "at end", message);
    } else {
        char str[100] = {0};
        snprintf(str, 100, "at '%s'", token->lexeme);
        report(token->line, str, message);
    }
}
