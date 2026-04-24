#include "token.h"
#include <stdbool.h>

static bool is_at_end(void) { return true; }
static void scan_token(void) {}
Token *scanner_scan_tokens(char const *const source, size_t *token_list_size) {
    int const initial_token_list_size = 10;
    Token *tokens = malloc(sizeof(Token) * initial_token_list_size);
    size_t current_token_position = 0;
    while (!is_at_end()) {
        scan_token();
    }
    Token *new_token = token_eof(0);
    tokens[current_token_position++] = *new_token;
    free(new_token);
    return tokens;
}
