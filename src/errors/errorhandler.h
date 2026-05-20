#include "../scanner/token.h"
#include <stdbool.h>

bool errorhandler_haderror(void);

void errorhandler_reseterrors(void);

void errorhandler_printerror(int line, char const message[static 1]);

void errorhandler_printerror_token(Token const *token,
                                   char const message[static 1]);
