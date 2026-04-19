#include <stdlib.h>

struct Token {
    int placeholder;
};

int tokenizer_scan_tokens(size_t len, struct Token tokens[len]);
