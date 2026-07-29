#include <dirent.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

typedef struct {
    char const *const filename;
    int expected_exit_code;
} TestCase;

bool test_file(char const *const binary_name, TestCase test) {
    printf("----------------------------------------\n");
    printf("Running %s %s...\n", binary_name, test.filename);
    char command[256] = {0};
    strncat(command, binary_name, 126);
    strcat(command, " ");
    strncat(command, test.filename, 127);
    int ret = system(command);

    if (ret == -1) {
        perror("Failed to create child process");
        return false;
    }

    int exit_code = WEXITSTATUS(ret);
    if (exit_code != test.expected_exit_code) {
        printf("FAILED: unexpected termination status %d (expected: %d).\n",
               exit_code, test.expected_exit_code);
        return false;
    }

    printf("PASSED\n");
    return true;
}

int main(int argc, char *argv[argc]) {
    if (argc != 2) {
        printf("Usage: jloxtest <filename>\n");
        return EXIT_FAILURE;
    }

    char const *const binary_name = argv[1];
    TestCase tests[] = {
        {"./tests/examples/print.lox", 0},
        {"./tests/examples/block_comments.lox", 0},
        {"./tests/examples/block_comments_nested.lox", 0},
        {"./tests/examples/addition_mismatch_error.lox", 1},
    };
    for (size_t i = 0; i < (sizeof(tests) / sizeof(tests[0])); ++i) {
        test_file(binary_name, tests[i]);
    }
}
