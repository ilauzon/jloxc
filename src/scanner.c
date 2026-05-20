#include "scanner.h"
#include "errorhandler.h"
#include "token.h"
#include "tokentype.h"
#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

static bool is_at_end(Scanner const *const scanner) {
    return scanner->current >= strlen(scanner->source);
}

static char advance(Scanner *const scanner) {
    return scanner->source[scanner->current++];
}

/**
 * @brief Add a token to the scanner's token list.
 *
 * @param scanner The scanner.
 * @param token The token to be added. This is shallow-copied into the scanner's
 * list, so the token may be freed after this call.
 */
static void add_token(Scanner *const scanner, Token *const token) {
    if (scanner->tokens_len == scanner->tokens_mem_size) {
        scanner->tokens_mem_size *= 2;
        scanner->tokens =
            realloc(scanner->tokens, scanner->tokens_mem_size * sizeof(Token));
    }
    memcpy(scanner->tokens + scanner->tokens_len++, token, sizeof(Token));
}

static void add_token_from_literal(Scanner *const scanner,
                                   enum TokenType const type,
                                   Literal const *const literal) {
    assert(scanner->start < scanner->current);

    size_t const lexeme_size = scanner->current - scanner->start;
    char *const lexeme = malloc(lexeme_size + 1);
    lexeme[lexeme_size] = '\0';

    strncpy(lexeme, scanner->source + scanner->start, lexeme_size);
    Token *token = token_init(type, lexeme, literal, scanner->line);
    add_token(scanner, token);
    free(token);
}

static void add_token_literal_from_type(Scanner *const scanner,
                                        enum TokenType const type) {
    add_token_from_literal(scanner, type, NULL);
}

/**
 * @brief Check if the current character matches an `expected` character, and
 * advances the scanner if so.
 *
 * @param scanner The scanner.
 * @param expected The expected character to match the current character
 * against.
 * @return True if the current character matches `expected`, else false.
 */
static bool match(Scanner *const scanner, char expected) {
    if (is_at_end(scanner)) {
        return false;
    }

    if (scanner->source[scanner->current] != expected) {
        return false;
    }

    scanner->current++;
    return true;
}

/**
 * @brief A lookahead. Return the current character without advancing to the
 * next character. Returns a null terminator if the scanner had reached the end
 * of its source code.
 *
 * @param scanner The scanner.
 * @return The character that the scanner is currently processing.
 */
static char peek(Scanner const *const scanner) {
    if (is_at_end(scanner)) {
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

/**
 * @brief Scan until the end of the current string lexeme is reached. Assumed to
 * be called within a string, i.e. with `current` pointing to a character after
 * the first `"` character.
 *
 * @param scanner The scanner.
 */
static void scan_string(Scanner *const scanner) {
    while (peek(scanner) != '"' && !is_at_end(scanner)) {
        if (peek(scanner) == '\n')
            scanner->line++;
        advance(scanner);
    }

    if (is_at_end(scanner)) {
        errorhandler_printerror(scanner->line, "Unterminated string.");
        return;
    }

    // consume the closing ".
    advance(scanner);

    // copy the string, trimming the surrounding quotes.
    assert(scanner->current > scanner->start);
    size_t string_length = scanner->current - scanner->start - 1;
    char *const string = malloc(string_length);
    memcpy(string, scanner->source + scanner->start + 1, string_length);
    string[string_length - 1] = '\0';

    Literal *const literal = token_literal_init(string);
    add_token_from_literal(scanner, TokenType_STRING, literal);
}

static void scan_number(Scanner *const scanner) {
    while (isdigit(peek(scanner))) {
        advance(scanner);
    }

    if (peek(scanner) == '.' && isdigit(scanner_peek_next(scanner))) {
        advance(scanner);

        while (isdigit(peek(scanner))) {
            advance(scanner);
        }
    }

    // copy the string containing the double
    assert(scanner->current > scanner->start);
    size_t string_length = scanner->current - scanner->start + 1;
    char *const string = malloc(string_length);
    memcpy(string, scanner->source + scanner->start, string_length);
    string[string_length - 1] = '\0';

    double value = atof(string);
    Literal *const literal = token_literal_init(value);
    add_token_from_literal(scanner, TokenType_NUMBER, literal);
}

static void scan_identifier(Scanner *const scanner) {
    while (isalnum(peek(scanner))) {
        advance(scanner);
    }

    // copy the string containing the identifier
    assert(scanner->current > scanner->start);
    size_t string_length = scanner->current - scanner->start + 1;
    char *const string = malloc(string_length);
    memcpy(string, scanner->source + scanner->start, string_length);
    string[string_length - 1] = '\0';

    enum TokenType type = tokentype_from_string(string);

    add_token_literal_from_type(scanner, type);
}

static void scan_block_comment(Scanner *const scanner) {
    char c;
    while (true) {
        c = advance(scanner);
        if (c == '\n') {
            scanner->line++;
        } else if (c == '/' && match(scanner, '*')) {
            scan_block_comment(scanner);
        } else if (c == '*') {
            bool comment_block_ended = match(scanner, '/');
            if (comment_block_ended) {
                break;
            }
        }
    }
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
static void scan_token(Scanner *const scanner) {
    char c = advance(scanner);
    switch (c) {
    // single-character lexemes
    case '(':
        add_token_literal_from_type(scanner, TokenType_LEFT_PAREN);
        break;
    case ')':
        add_token_literal_from_type(scanner, TokenType_RIGHT_PAREN);
        break;
    case '{':
        add_token_literal_from_type(scanner, TokenType_LEFT_BRACE);
        break;
    case '}':
        add_token_literal_from_type(scanner, TokenType_RIGHT_BRACE);
        break;
    case ',':
        add_token_literal_from_type(scanner, TokenType_COMMA);
        break;
    case '.':
        add_token_literal_from_type(scanner, TokenType_DOT);
        break;
    case '-':
        add_token_literal_from_type(scanner, TokenType_MINUS);
        break;
    case '+':
        add_token_literal_from_type(scanner, TokenType_PLUS);
        break;
    case ';':
        add_token_literal_from_type(scanner, TokenType_SEMICOLON);
        break;
    case '*':
        add_token_literal_from_type(scanner, TokenType_STAR);
        break;
    case '?':
        add_token_literal_from_type(scanner, TokenType_QUESTION);
        break;
    case ':':
        add_token_literal_from_type(scanner, TokenType_COLON);
        break;
    // two-character lexemes
    case '!':
        add_token_literal_from_type(scanner, match(scanner, '=')
                                                 ? TokenType_BANG_EQUAL
                                                 : TokenType_BANG);
        break;
    case '=':
        add_token_literal_from_type(scanner, match(scanner, '=')
                                                 ? TokenType_EQUAL_EQUAL
                                                 : TokenType_EQUAL);
        break;
    case '<':
        add_token_literal_from_type(scanner, match(scanner, '=')
                                                 ? TokenType_LESS_EQUAL
                                                 : TokenType_LESS);
        break;
    case '>':
        add_token_literal_from_type(scanner, match(scanner, '=')
                                                 ? TokenType_GREATER_EQUAL
                                                 : TokenType_GREATER);
        break;
    // comments
    case '/':
        if (match(scanner, '/')) {
            while (peek(scanner) != '\n' && !is_at_end(scanner)) {
                advance(scanner);
            }
        } else if (match(scanner, '*')) {
            scan_block_comment(scanner);
        } else {
            add_token_literal_from_type(scanner, TokenType_SLASH);
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
        scan_string(scanner);
        break;
    // error if nothing matches
    default:
        if (isdigit(c)) {
            scan_number(scanner);
        } else if (isalpha(c)) {
            scan_identifier(scanner);
        } else {
            errorhandler_printerror(scanner->line, "Unexpected character.");
        }
        break;
    }
}
static Scanner *init(char const *const source) {
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
 * @param source the string of source code.
 * @param token_list_size The number of tokens in the returned list.
 * @return A pointer to a list of the tokens scanned from the given source
 * code.
 */
Token *scanner_scan_tokens(char const *const source,
                           size_t *const token_list_size) {
    Scanner *scanner = init(source);
    while (!is_at_end(scanner)) {
        scan_token(scanner);
        scanner->start = scanner->current;
    }

    scanner->tokens_len++;
    scanner->tokens =
        realloc(scanner->tokens, scanner->tokens_len * sizeof(Token));

    Token eof = {
        .lexeme = "<EOF>",
        .line = scanner->line,
        .literal = NULL,
        .type = TokenType_EOF,
    };
    memcpy(scanner->tokens + scanner->tokens_len - 1, &eof, sizeof(Token));
    *token_list_size = scanner->tokens_len;
    Token *tokens = scanner->tokens;
    free(scanner);

    return tokens;
}
