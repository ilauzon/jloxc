#include "errorhandler.h"
#include <stdio.h>

static bool had_error = false;

void errorhandler_report(int line, char const where[static 1],
                         char const message[static 1]) {
    had_error = true;
    printf("[line %d] Error %s: %s\n", line, where, message);
}

bool errorhandler_haderror(void) { return had_error; }

void errorhandler_printerror(int line, char const message[static 1]) {
    errorhandler_report(line, "", message);
}
