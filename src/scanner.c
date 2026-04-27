#include "scanner.h"
#include "errorhandler.h"
#include "token.h"
#include "tokentype.h"
#include <assert.h>
#include <ctype.h>
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
 * @param scanner The scanner.
 * @param token The token to be added. This is shallow-copied into the scanner's
 * list, so the token may be freed after this call.
 */
void scanner_add_token(Scanner *const scanner, Token *const token) {
    if (scanner->tokens_len == scanner->tokens_mem_size) {
        scanner->tokens_mem_size *= 2;
        scanner->tokens =
            realloc(scanner->tokens, scanner->tokens_mem_size * sizeof(Token));
    }
    memcpy(scanner->tokens + scanner->tokens_len++, token, sizeof(Token));
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

/**
 * @brief Checks if the current character matches an `expected` character, and
 * advances the scanner if so.
 *
 * @param scanner The scanner.
 * @param expected The expected character to match the current character
 * against.
 * @return True if the current character matches `expected`, else false.
 */
static bool scanner_match(Scanner *const scanner, char expected) {
    if (scanner_is_at_end(scanner)) {
        return false;
    }

    if (scanner->source[scanner->current] != expected) {
        return false;
    }

    scanner->current++;
    return true;
}

/**
 * @brief A lookahead. Returns the current character without advancing to the
 * next character. Returns a null terminator if the scanner had reached the end
 * of its source code.
 *
 * @param scanner The scanner.
 * @return The character that the scanner is currently processing.
 */
static char scanner_peek(Scanner const *const scanner) {
    if (scanner_is_at_end(scanner)) {
        return '\0';
    }
    return scanner->source[scanner->current];
}

static char scanner_peek_next(Scanner const *const scanner) {
    if (scanner->current + 1 >= strlen(scanner->source) + 1) {
        return '\0';
    }
    return scanner->source[scanner->current + 1];
}

static char const *string_to_string(void const *const string,
                                    size_t const ignored) {
    return string;
}

static char const *number_to_string(void const *const number,
                                    size_t const ignored) {
    char *string = malloc(50);
    snprintf(string, 50, "%f", *(double *)number);
    return string;
}

/**
 * @brief Scan until the end of the current string lexeme is reached. Assumed to
 * be called within a string, i.e. with `current` pointing to a character after
 * the first `"` character.
 *
 * @param scanner The scanner.
 */
static void scanner_scan_string(Scanner *const scanner) {
    while (scanner_peek(scanner) != '"' && !scanner_is_at_end(scanner)) {
        if (scanner_peek(scanner) == '\n')
            scanner->line++;
        scanner_advance(scanner);
    }

    if (scanner_is_at_end(scanner)) {
        errorhandler_printerror(scanner->line, "Unterminated string.");
        return;
    }

    // consume the closing ".
    scanner_advance(scanner);

    // copy the string, trimming the surrounding quotes.
    assert(scanner->current > scanner->start);
    size_t string_length = scanner->current - scanner->start;
    char *const string = malloc(string_length);
    memcpy(string, scanner->source + scanner->start + 1, string_length - 2);
    string[string_length - 1] = '\0';

    Literal *const literal =
        token_literal_init(string, string_length, string_to_string);
    scanner_add_token_from_literal(scanner, TokenType_STRING, literal);
}

static void scanner_scan_number(Scanner *const scanner) {
    while (isdigit(scanner_peek(scanner))) {
        scanner_advance(scanner);
    }

    if (scanner_peek(scanner) == '.' && isdigit(scanner_peek_next(scanner))) {
        scanner_advance(scanner);

        while (isdigit(scanner_peek(scanner))) {
            scanner_advance(scanner);
        }
    }

    // copy the string containing the double
    assert(scanner->current > scanner->start);
    size_t string_length = scanner->current - scanner->start + 1;
    char *const string = malloc(string_length);
    memcpy(string, scanner->source + scanner->start, string_length);
    string[string_length - 1] = '\0';

    double *const value = malloc(sizeof(double));
    *value = atof(string);
    Literal *const literal =
        token_literal_init(value, sizeof(double), number_to_string);
    scanner_add_token_from_literal(scanner, TokenType_NUMBER, literal);
}

/**
 * @brief Scan in a single token from the current position of the scanner.
 *
 * The token is stored in the scanner's token list. If the current position is
 * some ignorable text, like comments or whitespace, the scanner will still be
 * advanced but no token is created.
 *
 * @param scanner The scanner.
 */
static void scanner_scan_token(Scanner *const scanner) {
    char c = scanner_advance(scanner);
    switch (c) {
    // single-character lexemes
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
    // two-character lexemes
    case '!':
        scanner_add_token_from_type(scanner, scanner_match(scanner, '=')
                                                 ? TokenType_BANG_EQUAL
                                                 : TokenType_BANG);
        break;
    case '=':
        scanner_add_token_from_type(scanner, scanner_match(scanner, '=')
                                                 ? TokenType_EQUAL_EQUAL
                                                 : TokenType_EQUAL);
        break;
    case '<':
        scanner_add_token_from_type(scanner, scanner_match(scanner, '=')
                                                 ? TokenType_LESS_EQUAL
                                                 : TokenType_LESS);
        break;
    case '>':
        scanner_add_token_from_type(scanner, scanner_match(scanner, '=')
                                                 ? TokenType_GREATER_EQUAL
                                                 : TokenType_GREATER);
        break;
    // > 2 character lexemes
    case '/':
        if (scanner_match(scanner, '/')) {
            while (scanner_peek(scanner) != '\n' &&
                   !scanner_is_at_end(scanner)) {
                scanner_advance(scanner);
            }
        } else {
            scanner_add_token_from_type(scanner, TokenType_SLASH);
        }
        break;
    // whitespace
    case ' ':
    case '\r':
    case '\t':
        break;
    case '\n':
        scanner->line++;
        break;
    case '"':
        scanner_scan_string(scanner);
        break;
    // error if nothing matches
    default:
        if (isdigit(c)) {
            scanner_scan_number(scanner);
        } else {
            errorhandler_printerror(scanner->line, "Unexpected character.");
        }
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
