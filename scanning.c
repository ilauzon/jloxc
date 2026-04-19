#include "tokenizer.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
    MAX_LINE_SIZE = 500,
    MAX_TOKENS_PER_LINE = 500,
};

int run(char const line[static 1]) {
    struct Token tokens[MAX_TOKENS_PER_LINE] = {0};

    if (line == NULL) {
        // figure out a better way to handle a NP error here
        return EXIT_FAILURE;
    }

    int tokens_scanned = tokenizer_scan_tokens(MAX_TOKENS_PER_LINE, tokens);

    for (int i = 0; i < tokens_scanned; ++i) {
        printf("%d", tokens[i].placeholder);
    }

    return EXIT_SUCCESS;
}

int run_file(char const path[static 1]) {
    FILE *file = fopen(path, "r");
    if (!file) {
        perror("fopen failed");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int run_prompt() {
    while (true) {
        printf("> ");
        char line[MAX_LINE_SIZE] = {0};
        scanf("%s", line);
        if (!strlen(line)) {
            break;
        }
        run(line);
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
