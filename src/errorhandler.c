#include <stdbool.h>
#include <stdio.h>

void errorhandler_report(int line, char const where[static 1],
                         char const message[static 1]) {
    printf("[line %d] Error %s: %s\n", line, where, message);
}

void errorhandler_printerror(int line, char const message[static 1],
                             bool *had_error) {
    *had_error = true;
    errorhandler_report(line, "", message);
}
