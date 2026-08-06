#include "main.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
    printf("Running %ld unit tests...\n", sizeof(tests) / sizeof(tests[0]));
    printf("==========================================\n");
    for (size_t i = 0; i < sizeof(tests) / sizeof(tests[0]); ++i) {
        TestResult result = tests[i]();
        if (result.code != 0) {
            printf(" ❌ %s failed with code %s:\n    %s\n", result.name,
                   testresultcode_to_string(result.code), result.message);
        } else {
            printf(" ✅ %s passed\n", result.name);
        }
        free(result.message);
    }
    printf("=============== DONE TESTS ===============\n");
}
