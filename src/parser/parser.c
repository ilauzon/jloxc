#include "../errorhandler.h"
#include "../token.h"
#include "../tokentype.h"
#include "expr.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>

typedef struct {
    Token const *tokens;
    size_t tokens_len;
    int current;
} Parser;

static Parser *init(Token const *const tokens, size_t const tokens_len) {
    Parser *p = calloc(1, sizeof(Parser));
    p->tokens = tokens;
    p->tokens_len = tokens_len;
    p->current = 0;
    return p;
}

/**
 * @brief Get the last consumed token.
 *
 * @return The last token consumed by the parser.
 */
static Token const *previous(Parser *parser) {
    return parser->tokens + parser->current - 1;
}

/**
 * @brief Get the current token without consuming it.
 *
 * @return The current token the parser is on, i.e. the first one it hasn't
 * consumed.
 */
static Token const *peek(Parser *parser) {
    return parser->tokens + parser->current;
}

/**
 * @brief Check if there are no more tokens to parse.
 *
 * @return True if the token the parser currently on is `TokenType_EOF`, false
 * otherwise.
 */
static bool is_at_end(Parser *parser) {
    return peek(parser)->type == TokenType_EOF;
}

/**
 * @brief Consume the current token.
 *
 * @return The consumed token.
 */
static Token const *advance(Parser *parser) {
    if (!is_at_end(parser)) {
        parser->current++;
    }
    return previous(parser);
}

/**
 * @brief Check if the parser's current token is a particular type.
 *
 * @param type The type to compare against.
 * @return False if the current token is `TokenType_EOF` or if `type` does not
 * equal the current token's type, true otherwise.
 */
static bool check(Parser *parser, enum TokenType type) {
    if (is_at_end(parser)) {
        return false;
    }
    return peek(parser)->type == type;
}

/**
 * @brief Consume the current token and report an error if it does not match the
 * given type.
 *
 * @param type The type to match against.
 * @param message The message to print if the type of the current token is not
 * `type`.
 * @returns The consumed token.
 */
static Token const *consume(Parser *parser, enum TokenType type,
                            char const *const message) {
    Token const *token = peek(parser);
    if (token->type == type) {
        return advance(parser);
    }
    errorhandler_printerror(token->line, message);
    return NULL;
}

/**
 * @brief Check if the parser's current token matches one in a set of types.
 *
 * @param arg_count the number of types being passed in.
 * @return False if the current token is `TokenType_EOF` or if the current
 * token's type does not match any of the given types, true otherwise.
 */
static bool match(Parser *parser, int arg_count, ...) {
    va_list types;
    va_start(types, arg_count);
    for (int i = 0; i < arg_count; i++) {
        enum TokenType type = va_arg(types, enum TokenType);
        if (check(parser, type)) {
            advance(parser);
            return true;
        }
    }

    return false;
}

static Expr *expression(Parser *parser);

static Expr *primary(Parser *parser) {
    if (match(parser, 3, TokenType_FALSE, TokenType_TRUE, TokenType_NIL)) {
        return (Expr *)expr_init_literal(previous(parser)->literal);
    }

    if (match(parser, 2, TokenType_NUMBER, TokenType_STRING)) {
        return (Expr *)expr_init_literal(previous(parser)->literal);
    }

    if (match(parser, 1, TokenType_LEFT_PAREN)) {
        Expr *expr = expression(parser);
        consume(parser, TokenType_RIGHT_PAREN, "Expect ')' after expression.");
        return (Expr *)expr_init_grouping(expr);
    }

    errorhandler_printerror(
        parser->tokens[parser->current].line,
        "Reached end of parser unexpectedly; primary production failed.");
    /*
     * is this the right thing to return if nothing
     * matches in this root case?
     */
    return NULL;
}
static Expr *unary(Parser *parser) {
    if (match(parser, 2, TokenType_BANG, TokenType_MINUS)) {
        Token const *operator = previous(parser);
        Expr *expr = unary(parser);
        return (Expr *)expr_init_unary(operator, expr);
    }
    return primary(parser);
}
static Expr *factor(Parser *parser) {
    Expr *expr = unary(parser);
    while (match(parser, 2, TokenType_SLASH, TokenType_STAR)) {
        Token const *operator = previous(parser);
        Expr *right = unary(parser);
        expr = (Expr *)expr_init_binary(expr, operator, right);
    }
    return expr;
}
static Expr *term(Parser *parser) {
    Expr *expr = factor(parser);
    while (match(parser, 2, TokenType_PLUS, TokenType_MINUS)) {
        Token const *operator = previous(parser);
        Expr *right = factor(parser);
        expr = (Expr *)expr_init_binary(expr, operator, right);
    }
    return expr;
}
static Expr *comparison(Parser *parser) {
    Expr *expr = term(parser);
    while (match(parser, 4, TokenType_GREATER, TokenType_GREATER_EQUAL,
                 TokenType_LESS_EQUAL, TokenType_LESS)) {
        Token const *operator = previous(parser);
        Expr *right = term(parser);
        expr = (Expr *)expr_init_binary(expr, operator, right);
    }
    return expr;
}
static Expr *equality(Parser *parser) {
    Expr *expr = comparison(parser);
    while (match(parser, 2, TokenType_BANG_EQUAL, TokenType_EQUAL_EQUAL)) {
        Token const *operator = previous(parser);
        Expr *right = comparison(parser);
        expr = (Expr *)expr_init_binary(expr, operator, right);
    }
    return expr;
}
static Expr *expression(Parser *parser) { return equality(parser); }

Expr *parser_parse_expression(Token const *const tokens,
                              size_t const tokens_len) {
    Parser *parser = init(tokens, tokens_len);
    return expression(parser);
}
