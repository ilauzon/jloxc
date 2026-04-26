#include "scanner.h"
#include "errorhandler.h"
#include "token.h"
#include "tokentype.h"
#include <assert.h>
#include <stdbool.h>

static bool scanner_is_at_end(Scanner const *const scanner) {
    return scanner->current >= strlen(scanner->source);
}

static char scanner_advance(Scanner *const scanner) {
    return scanner->source[scanner->current++];
}

/**
 * @brief Add a token to the scanner's token list.
 *
 * @param scanner
 * @param token The token to be added. This is shallow-copied into the scanner's
 * list, so the token may be freed after this call.
 */
void scanner_add_token(Scanner *const scanner, Token *const token) {
    if (scanner->tokens_len == scanner->tokens_mem_size) {
        scanner->tokens_mem_size *= 2;
        scanner->tokens =
            realloc(scanner->tokens, scanner->tokens_mem_size * sizeof(Token));
    }
    memcpy(scanner->tokens + (scanner->tokens_len++ * sizeof(Token)), token,
           sizeof(Token));
}

static void scanner_add_token_from_literal(Scanner *const scanner,
                                           enum TokenType const type,
                                           Literal const *const literal) {
    assert(scanner->start < scanner->current);

    size_t const lexeme_size = scanner->current - scanner->start;
    char *const lexeme = malloc(lexeme_size + 1);
    if (!lexeme) {
        perror("Memory allocation failed");
        return;
    }
    lexeme[lexeme_size] = '\0';

    strncpy(lexeme, scanner->source + scanner->start, lexeme_size);
    Token *token = token_init(type, lexeme, literal, scanner->line);
    scanner_add_token(scanner, token);
    free(token);
}

static void scanner_add_token_from_type(Scanner *const scanner,
                                        enum TokenType const type) {
    scanner_add_token_from_literal(scanner, type, NULL);
}

static void scanner_scan_token(Scanner *const scanner) {
    char c = scanner_advance(scanner);
    switch (c) {
    case '(':
        scanner_add_token_from_type(scanner, TokenType_LEFT_PAREN);
        break;
    case ')':
        scanner_add_token_from_type(scanner, TokenType_RIGHT_PAREN);
        break;
    case '{':
        scanner_add_token_from_type(scanner, TokenType_LEFT_BRACE);
        break;
    case '}':
        scanner_add_token_from_type(scanner, TokenType_RIGHT_BRACE);
        break;
    case ',':
        scanner_add_token_from_type(scanner, TokenType_COMMA);
        break;
    case '.':
        scanner_add_token_from_type(scanner, TokenType_DOT);
        break;
    case '-':
        scanner_add_token_from_type(scanner, TokenType_MINUS);
        break;
    case '+':
        scanner_add_token_from_type(scanner, TokenType_PLUS);
        break;
    case ';':
        scanner_add_token_from_type(scanner, TokenType_SEMICOLON);
        break;
    case '*':
        scanner_add_token_from_type(scanner, TokenType_STAR);
        break;
    default:
        errorhandler_printerror(scanner->line, "Unexpected character.");
        break;
    }
}

Scanner *scanner_init(char const *const source) {
    size_t initial_token_list_size = 10;
    Token *token_list = calloc(initial_token_list_size, sizeof(Token));
    Scanner scanner_init = {
        .source = source,
        .start = 0,
        .current = 0,
        .line = 1,
        .tokens = token_list,
        .tokens_len = 0,
        .tokens_mem_size = initial_token_list_size,
    };

    Scanner *scanner = calloc(1, sizeof(Scanner));
    if (!scanner) {
        perror("Memory allocation failed");
        return NULL;
    }
    memcpy(scanner, &scanner_init, sizeof(Scanner));
    return scanner;
}

/**
 * @brief Scan the given source code string and convert it to a list of
 * tokens.
 *
 * @param scanner The scanner to use.
 * @param token_list_size The number of tokens in the returned list.
 * @return A pointer to a list of the tokens scanned from the given source
 * code.
 */
Token *scanner_scan_tokens(Scanner *const scanner,
                           size_t *const token_list_size) {
    while (!scanner_is_at_end(scanner)) {
        scanner_scan_token(scanner);
        scanner->start = scanner->current;
    }
    *token_list_size = scanner->tokens_len;
    return scanner->tokens;
}
