#include "./parser/ast.h"
#include "./parser/expr.h"
#include "errorhandler.h"
#include "scanner.h"
#include "token.h"
#include "tokentype.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
    MAX_LINE_SIZE = 500,
    MAX_TOKENS_PER_LINE = 500,
};

void run(char const line[static 1]) {
    size_t token_list_size = 0;
    Token *tokens = scanner_scan_tokens(line, &token_list_size);
    printf("Number of tokens scanned: %ld\n", token_list_size);
    for (int i = 0; i < (long)token_list_size; i++) {
        Token const *const token = tokens + i;
        enum TokenType type = token->type;
        Literal const *const literal = token->literal;

        char const *literal_value = "<none>";
        if (literal) {
            literal_value = literal->to_string(literal);
        }

        printf("[Token %d]: \n\ttype: \t%s "
               "\n\tlex:\t%s\n\tlit: \t%s\n\tline: \t%d\n",
               i, tokentype_to_string(type), token->lexeme, literal_value,
               token->line);
    }
}

int run_file(char const path[static 1]) {
    FILE *file = fopen(path, "r");
    if (!file) {
        char error_msg[100] = {0};
        snprintf(error_msg, 100, "Failed to open file %s.", path);
        errorhandler_printerror(1, error_msg);
        return EXIT_FAILURE;
    }

    fseek(file, 0, SEEK_END);
    long const file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    char *const contents = malloc(file_size + 1);
    fread(contents, file_size, 1, file);
    fclose(file);

    run(contents);
    if (errorhandler_haderror()) {
        return EXIT_FAILURE;
    }

    free(contents);
    return EXIT_SUCCESS;
}

int run_prompt(void) {
    while (true) {
        printf("> ");
        char line[MAX_LINE_SIZE] = {0};
        char const *const ret = fgets(line, sizeof(line), stdin);
        if (ret == NULL || !strlen(line)) {
            break;
        }
        run(line);
        if (errorhandler_haderror()) {
            return EXIT_FAILURE;
        }
    }
    return EXIT_SUCCESS;
}

void run_example_ast_print() {
    int literal_1 = 123;
    double literal_2 = 45.67;
    ExprBinary *expression = expr_init_binary(
        (Expr *)expr_init_unary(
            token_init(TokenType_MINUS, "-", NULL, 1),
            (Expr *)expr_init_literal(token_literal_init(literal_1))),
        token_init(TokenType_STAR, "*", NULL, 1),
        (Expr *)expr_init_grouping(
            (Expr *)expr_init_literal(token_literal_init(literal_2))));
    ExprVisitor *printer = ast_init_printer();
    char const *str = printer->visit_binary(expression);
    printf("%s\n", str);
}

int main(int argc, char *argv[argc + 1]) {
    // run_example_ast_print();
    if (argc > 2) {
        printf("Usage: jlox [script]\n");
        return EXIT_FAILURE;
    } else if (argc == 2) {
        return run_file(argv[1]);
    } else {
        return run_prompt();
    }
}
