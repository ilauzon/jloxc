#pragma once
#include "token.h"
#include <stdlib.h>

Token *scanner_scan_tokens(char const *const source,
                           size_t *const token_list_size);
