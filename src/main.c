#include "errors/errorhandler.h"
#include "interpreter/interpreter.h"
#include "parser/parser.h"
#include "scanner/scanner.h"
#include "scanner/token.h"
#include "scanner/tokentype.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
    MAX_LINE_SIZE = 500,
    MAX_TOKENS_PER_LINE = 500,
};

static void print_tokens(size_t tokens_len, Token tokens[tokens_len]) {
    printf("Number of tokens scanned: %ld\n", tokens_len);
    for (size_t i = 0; i < tokens_len; i++) {
        Token const *const token = tokens + i;
        enum TokenType type = token->type;
        Literal const *const literal = token->literal;

        char const *literal_value = "<none>";
        if (literal) {
            literal_value = literal->to_string(literal);
        }

        printf("[Token %ld]: \n\ttype: \t%s "
               "\n\tlex:\t%s\n\tlit: \t%s\n\tline: \t%d\n",
               i, tokentype_to_string(type), token->lexeme, literal_value,
               token->line);

        if (literal) {
            free((char *)literal_value);
        }
    }
}

void run(char const line[static 1]) {
    size_t token_list_size = 0;
    Token *tokens = scanner_scan_tokens(line, &token_list_size);

    if (errorhandler_haderror()) {
        return;
    }

    print_tokens(token_list_size, tokens);

    size_t stmts_len = 0;
    Stmt *stmts = parser_parse(tokens, token_list_size, &stmts_len);
    for (size_t i = 0; i < token_list_size; ++i) {
        Token *t = tokens + i;
        free((char *)t->lexeme);
        if (t->literal != NULL) {
            free((void *)t->literal);
        }
    }
    free(tokens);

    if (errorhandler_haderror()) {
        return;
    }

    Interpreter *interpreter = interpreter_init();
    interpreter_interpret(interpreter, stmts_len, stmts);
    free(stmts);
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
    contents[file_size] = '\0';
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
        errorhandler_reseterrors();
    }
    return EXIT_SUCCESS;
}

int main(int argc, char *argv[argc + 1]) {
    if (argc > 2) {
        printf("Usage: jlox [script]\n");
        return EXIT_FAILURE;
    } else if (argc == 2) {
        return run_file(argv[1]);
    } else {
        return run_prompt();
    }
}
