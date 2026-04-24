#include "errorhandler.h"
#include "tokenizer.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
    MAX_LINE_SIZE = 500,
    MAX_TOKENS_PER_LINE = 500,
};

void run(char const line[static 1], bool *had_error) {
    Token tokens[MAX_TOKENS_PER_LINE] = {0};
    int tokens_scanned = tokenizer_scan_tokens(MAX_TOKENS_PER_LINE, tokens);
    for (int i = 0; i < tokens_scanned; ++i) {
        // printf("%d", tokens[i].placeholder);
    }
    *had_error = false;
}

int run_file(char const path[static 1]) {
    FILE *file = fopen(path, "r");
    if (!file) {
        char error_msg[100] = {0};
        snprintf(error_msg, 100, "Failed to open file %s.", path);
        bool _;
        errorhandler_printerror(1, error_msg, &_);
        return EXIT_FAILURE;
    }

    fseek(file, 0, SEEK_END);
    long const file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    char *const contents = malloc(file_size + 1);
    fread(contents, file_size, 1, file);
    fclose(file);

    bool had_error = false;
    run(contents, &had_error);
    if (had_error) {
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
        bool had_error = false;
        run(line, &had_error);
        if (had_error) {
            return EXIT_FAILURE;
        }
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
