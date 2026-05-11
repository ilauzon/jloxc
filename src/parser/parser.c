#include "parser.h"
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

/**
 * @brief Flag for panic mode, where the parser will unwind until reaching a
 * synchronization point.
 */
static bool panic_mode = false;

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

static void error(Token const *const token, char const *const message) {
    errorhandler_printerror_token(token, message);
    panic_mode = true;
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
    error(token, message);
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
    Token const *const prev = previous(parser);
    if (match(parser, 6, TokenType_FALSE, TokenType_TRUE, TokenType_NIL,
              TokenType_NUMBER, TokenType_STRING, TokenType_IDENTIFIER)) {
        void const *value_ptr = NULL;
        if (prev->literal) {
            value_ptr = prev->literal->value;
        }
        return (Expr *)expr_init_literal(prev->type, value_ptr);
    }

    if (match(parser, 1, TokenType_LEFT_PAREN)) {
        Expr *expr = expression(parser);
        consume(parser, TokenType_RIGHT_PAREN, "Expect ')' after expression.");
        if (panic_mode) {
            return NULL;
        }
        return (Expr *)expr_init_grouping(expr);
    }
    error(peek(parser), "Expect expression");
    return NULL;
}

static Expr *unary(Parser *parser) {
    if (match(parser, 2, TokenType_BANG, TokenType_MINUS)) {
        Token const *operator = previous(parser);
        Expr *expr = unary(parser);
        if (panic_mode) {
            return NULL;
        }
        return (Expr *)expr_init_unary(operator, expr);
    }
    return primary(parser);
}

static Expr *factor(Parser *parser) {
    Expr *expr = unary(parser);
    if (panic_mode) {
        return NULL;
    }
    while (match(parser, 2, TokenType_SLASH, TokenType_STAR)) {
        Token const *operator = previous(parser);
        Expr *right = unary(parser);
        if (panic_mode) {
            return NULL;
        }
        expr = (Expr *)expr_init_binary(expr, operator, right);
    }
    return expr;
}

static Expr *term(Parser *parser) {
    Expr *expr = factor(parser);
    if (panic_mode) {
        return NULL;
    }
    while (match(parser, 2, TokenType_PLUS, TokenType_MINUS)) {
        Token const *operator = previous(parser);
        Expr *right = factor(parser);
        if (panic_mode) {
            return NULL;
        }
        expr = (Expr *)expr_init_binary(expr, operator, right);
    }
    return expr;
}

static Expr *comparison(Parser *parser) {
    Expr *expr = term(parser);
    if (panic_mode) {
        return NULL;
    }
    while (match(parser, 4, TokenType_GREATER, TokenType_GREATER_EQUAL,
                 TokenType_LESS_EQUAL, TokenType_LESS)) {
        Token const *operator = previous(parser);
        Expr *right = term(parser);
        if (panic_mode) {
            return NULL;
        }
        expr = (Expr *)expr_init_binary(expr, operator, right);
    }
    return expr;
}

static Expr *equality(Parser *parser) {
    Expr *expr = comparison(parser);
    if (panic_mode) {
        return NULL;
    }
    while (match(parser, 2, TokenType_BANG_EQUAL, TokenType_EQUAL_EQUAL)) {
        Token const *operator = previous(parser);
        Expr *right = comparison(parser);
        if (panic_mode) {
            return NULL;
        }
        expr = (Expr *)expr_init_binary(expr, operator, right);
    }
    return expr;
}

static Expr *expression(Parser *parser) { return equality(parser); }

static void synchronize(Parser *parser) {
    panic_mode = false;
    advance(parser);
    while (!is_at_end(parser)) {
        if (previous(parser)->type == TokenType_SEMICOLON) {
            return;
        }
        switch (peek(parser)->type) {
        case TokenType_CLASS:
        case TokenType_FUN:
        case TokenType_VAR:
        case TokenType_FOR:
        case TokenType_IF:
        case TokenType_WHILE:
        case TokenType_PRINT:
        case TokenType_RETURN:
            return;
        default:
            break;
        }
        advance(parser);
    }
}

Expr *parser_parse(Token const *const tokens, size_t const tokens_len) {
    Parser *parser = init(tokens, tokens_len);
    Expr *expr = expression(parser);
    return expr;
}
