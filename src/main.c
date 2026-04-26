#include "errorhandler.h"
#include "token.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
    MAX_LINE_SIZE = 500,
    MAX_TOKENS_PER_LINE = 500,
};

void run(char const line[static 1]) {
    Token tokens[MAX_TOKENS_PER_LINE] = {0};
    // int tokens_scanned = scanner_scan_tokens();
    // for (int i = 0; i < tokens_scanned; ++i) {
    //     // printf("%d", tokens[i].placeholder);
    // }
}

int run_file(char const path[static 1]) {
    FILE *file = fopen(path, "r");
    if (!file) {
        char error_msg[100] = {0};
        snprintf(error_msg, 100, "Failed to open file %s.", path);
        bool _;
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
