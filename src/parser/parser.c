/**
 * Contains productions for parsing a string of tokens.
 *
 * Expression grammar:
 *
 * program            -> declaration* EOF ;
 * declaration        -> varDecl
 *                     | statement ;
 * varDecl            -> "var" IDENTIFIER ( "=" expression )? ";" ;
 * statement          -> exprStmt
 *                     | printStmt ;
 * exprStmt           -> expression ";" ;
 * printStmt          -> "print" expression ";" ;
 * expression         -> comma ;
 * comma              -> conditional ( "," conditional )* ;
 * conditional        -> expression ? expression : conditional
 *                     | expression ;
 * equality           -> comparison ( equality_rhs )*
 *                     | equality_rhs ;
 * equality_rhs       -> ( "!=" | "==" ) comparison ;
 * comparison         -> term ( comparison_rhs )*
 *                     | comparison_rhs ;
 * comparison_rhs     -> ( ">" | ">=" | "<" | "<=" ) term ;
 * term               -> factor ( term_rhs )*
 *                     | term_rhs ;
 * term_rhs           -> ( "-" | "+" ) factor ;
 * factor             -> unary ( factor_rhs )*
 *                     | factor_rhs;
 * factor_rhs         -> ( "/" | "*" ) unary ;
 * unary              -> ( "!" | "-" ) unary
 *                     | conditional ;
 * primary            -> "true" | "false" | "nil"
 *                     | NUMBER | STRING
 *                     | "(" expression ")"
 *                     | IDENTIFIER ;
 */

#include "parser.h"
#include "../errors/errorhandler.h"
#include "../scanner/token.h"
#include "../scanner/tokentype.h"
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
static Token const *peek(Parser const *const parser) {
    return parser->tokens + parser->current;
}

/**
 * @brief Check if there are no more tokens to parse.
 *
 * @return True if the token the parser currently on is `TokenType_EOF`, false
 * otherwise.
 */
static bool is_at_end(Parser const *const parser) {
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

static void error(Token const *const token, char const *const message) {
    errorhandler_printerror_token(token, message);
    panic_mode = true;
}

/**
 * @brief Consume the current token, then report an error if it does not match
 * the given type.
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
 * @brief Check if the parser's current token is of a certain type.
 *
 * @param type the token type to check against.
 * @return False if the current token is `TokenType_EOF` or if the current
 * token's type does not match the given type, true otherwise.
 */
static bool check_single(Parser const *const parser,
                         enum TokenType const type) {
    if (is_at_end(parser)) {
        return false;
    }

    return (peek(parser)->type == type);
}

/**
 * @brief Check if the parser's current token matches one in a set of types.
 *
 * @param arg_count the number of types being passed in.
 * @return False if the current token is `TokenType_EOF` or if the current
 * token's type does not match any of the given types, true otherwise.
 */
static bool check(Parser const *const parser, int const arg_count, ...) {
    if (is_at_end(parser)) {
        return false;
    }

    va_list types;
    va_start(types, arg_count);
    for (int i = 0; i < arg_count; i++) {
        enum TokenType type = va_arg(types, enum TokenType);
        if (check_single(parser, type)) {
            return true;
        }
    }

    return false;
}

/**
 * @brief Check if the parser's current token matches one in a set of types, and
 * advance if this is the case.
 *
 * @param arg_count the number of types being passed in.
 * @return False if the current token is `TokenType_EOF` or if the current
 * token's type does not match any of the given types, true otherwise.
 */
static bool match(Parser *const parser, int const arg_count, ...) {
    va_list types;
    va_start(types, arg_count);
    for (int i = 0; i < arg_count; i++) {
        enum TokenType type = va_arg(types, enum TokenType);
        if (check_single(parser, type)) {
            advance(parser);
            return true;
        }
    }
    return false;
}

static bool type_is_in_list(enum TokenType type, size_t size,
                            enum TokenType list[size]) {
    for (size_t i = 0; i < size; ++i) {
        if (list[i] == type) {
            return true;
        }
    }
    return false;
}

/**
 * @brief Parse a left-to-right associativity binary expression.
 *
 * @param next_production the next higher precedence production.
 * @param parser the parser being used.
 * @param operator_count the number of interchangeable operators in this
 * production.
 * @return A binary expression.
 */
static Expr *parse_left_assoc_binary(Expr *(*next_production)(Parser *parser),
                                     Parser *parser, int const operator_count,
                                     ...) {
    // parse operator token types into list
    enum TokenType *types_to_check =
        calloc(operator_count, sizeof(enum TokenType));
    va_list types;
    va_start(types, operator_count);
    for (int i = 0; i < operator_count; ++i) {
        enum TokenType type = va_arg(types, enum TokenType);
        types_to_check[i] = type;
    }

    // check if the parser is on the operator right now (i.e. the user forgot
    // the left-hand operand), if so emit an error.

    Token const *current = peek(parser);
    if (type_is_in_list(current->type, operator_count, types_to_check)) {
        Expr *left = (Expr *)expr_init_missing();
        Token const *operator = advance(parser);
        Expr *right = next_production(parser);

        if (panic_mode) {
            free(types_to_check);
            free(left);
            free(right);
            return NULL;
        }

        Expr *expr = (Expr *)expr_init_binary(left, operator, right);
        error(operator, "binary operator missing left-hand operand");
        free(types_to_check);
        free(left);
        free(right);
        free(expr);
        return NULL;
    }

    Expr *expr = next_production(parser);
    if (panic_mode) {
        free(types_to_check);
        free(expr);
        return NULL;
    }

    while (
        type_is_in_list(peek(parser)->type, operator_count, types_to_check)) {
        Token const *operator = advance(parser);
        Expr *right = next_production(parser);
        if (panic_mode) {
            free(types_to_check);
            free(expr);
            free(right);
            return NULL;
        }
        expr = (Expr *)expr_init_binary(expr, operator, right);
        if (panic_mode) {
            free(types_to_check);
            free(expr);
            free(right);
            return NULL;
        }
    }

    free(types_to_check);
    return expr;
}

static Expr *expression(Parser *parser);

static Expr *primary(Parser *parser) {
    if (expr_token_is_literal(peek(parser))) {
        Token const *const current = advance(parser);
        return expr_init_literal(current);
    }

    if (match(parser, 1, TokenType_IDENTIFIER)) {
        return expr_init_var(previous(parser));
    }

    if (match(parser, 1, TokenType_LEFT_PAREN)) {
        Expr *expr = expression(parser);
        consume(parser, TokenType_RIGHT_PAREN, "Expect ')' after expression.");
        if (panic_mode) {
            free(expr);
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
            free(expr);
            return NULL;
        }
        return (Expr *)expr_init_unary(operator, expr);
    }
    return primary(parser);
}

static Expr *factor(Parser *parser) {
    return parse_left_assoc_binary(unary, parser, 2, TokenType_SLASH,
                                   TokenType_STAR);
}

static Expr *term(Parser *parser) {
    return parse_left_assoc_binary(factor, parser, 2, TokenType_PLUS,
                                   TokenType_MINUS);
}

static Expr *comparison(Parser *parser) {
    return parse_left_assoc_binary(term, parser, 4, TokenType_GREATER,
                                   TokenType_GREATER_EQUAL,
                                   TokenType_LESS_EQUAL, TokenType_LESS);
}

static Expr *equality(Parser *parser) {
    return parse_left_assoc_binary(comparison, parser, 2, TokenType_BANG_EQUAL,
                                   TokenType_EQUAL_EQUAL);
}

static Expr *conditional(Parser *parser) {
    Expr *expr = equality(parser);
    if (panic_mode) {
        free(expr);
        return NULL;
    }

    if (check(parser, 1, TokenType_QUESTION)) {
        Token const *left_operator = advance(parser);
        Expr *middle_expr = equality(parser);
        if (panic_mode) {
            free(expr);
            free(middle_expr);
            return NULL;
        }
        consume(parser, TokenType_COLON,
                "Expect ':' after '?' in ternary conditional.");
        if (panic_mode) {
            free(expr);
            free(middle_expr);
            return NULL;
        }
        Token const *right_operator = previous(parser);
        Expr *right_expr = conditional(parser);
        if (panic_mode) {
            free(expr);
            free(middle_expr);
            free(right_expr);
            return NULL;
        }
        expr = (Expr *)expr_init_ternary(expr, left_operator, middle_expr,
                                         right_operator, right_expr);
    }
    return expr;
}

static Expr *comma(Parser *parser) {
    Expr *expr = conditional(parser);
    if (panic_mode) {
        free(expr);
        return NULL;
    }
    while (match(parser, 1, TokenType_COMMA)) {
        Token const *operator = previous(parser);
        Expr *right = conditional(parser);
        if (panic_mode) {
            free(expr);
            free(right);
            return NULL;
        }
        expr = (Expr *)expr_init_binary(expr, operator, right);
    }
    return expr;
}

static Expr *expression(Parser *parser) { return comma(parser); }

static Stmt *expression_statement(Parser *parser) {
    Expr *value = expression(parser);
    consume(parser, TokenType_SEMICOLON, "Expect ';' after value.");
    if (panic_mode) {
        free(value);
        return NULL;
    }
    Stmt *stmt = stmt_expr_init(value);
    free(value);
    return stmt;
}

static Stmt *print_statement(Parser *parser) {
    Expr *value = expression(parser);
    consume(parser, TokenType_SEMICOLON, "Expect ';' after value.");
    if (panic_mode) {
        free(value);
        return NULL;
    }
    Stmt *stmt = stmt_print_init(value);
    free(value);
    return stmt;
}

static Stmt *statement(Parser *parser) {
    if (match(parser, 1, TokenType_PRINT)) {
        return print_statement(parser);
    }
    return expression_statement(parser);
}

static Stmt *varDecl(Parser *parser) {
    Token const *const name =
        consume(parser, TokenType_IDENTIFIER, "Expect variable name.");
    if (panic_mode) {
        return NULL;
    }
    Expr *value = NULL;
    if (match(parser, 1, TokenType_EQUAL)) {
        value = expression(parser);
        if (panic_mode) {
            free(value);
            return NULL;
        }
    }

    consume(parser, TokenType_SEMICOLON, "Expect ';' after value.");
    if (panic_mode) {
        free(value);
        return NULL;
    }

    Stmt *stmt = stmt_var_init(name->lexeme, value);
    if (panic_mode) {
        free(value);
        free(stmt);
        return NULL;
    }
    return stmt;
}

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

static Stmt *declaration(Parser *parser) {
    Stmt *stmt;
    if (match(parser, 1, TokenType_VAR)) {
        stmt = varDecl(parser);
    } else {
        stmt = statement(parser);
    }

    if (panic_mode) {
        synchronize(parser);
    }

    return stmt;
}

Stmt *parser_parse(Token const *const tokens, size_t const tokens_len,
                   size_t *return_length_ptr) {
    Parser *parser = init(tokens, tokens_len);
    Stmt *statements = NULL;

    *return_length_ptr = 0;

    while (!is_at_end(parser)) {
        (*return_length_ptr)++;
        statements = realloc(statements, *return_length_ptr * sizeof(Stmt));
        Stmt *stmt = declaration(parser);

        if (stmt != NULL) {
            statements[*return_length_ptr - 1] = *stmt;
            free(stmt);
        }
    }
    free(parser);
    return statements;
}
