#include "../../src/arena_allocator/allocator.h"
#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

TestResult test_allocator_allocate() {
    CREATE_TESTRESULT()
    ArenaAllocator *allocator = arena_init();
    double *value_d = arena_allocate(allocator, sizeof(double));
    double ref_d = 5481;
    *value_d = ref_d;
    int *value_i = arena_allocate(allocator, sizeof(int));
    int ref_i = 5481;
    *value_i = ref_i;
    char *value_c = arena_allocate(allocator, sizeof(char));
    char ref_c = 'T';
    *value_c = ref_c;

    if (*value_d != ref_d) {
        ret.code = TestResultCode_FAILED_TEST;
        char msg[100] = {0};
        snprintf(msg, 99, "reference %f does not match %f allocated by arena",
                 ref_d, *value_d);
    }
    if (*value_i != ref_i) {
        ret.code = TestResultCode_FAILED_TEST;
        char msg[100] = {0};
        snprintf(msg, 99, "reference %d does not match %d allocated by arena",
                 ref_i, *value_i);
    }
    if (*value_c != ref_c) {
        ret.code = TestResultCode_FAILED_TEST;
        char msg[100] = {0};
        snprintf(msg, 99, "reference %c does not match %c allocated by arena",
                 ref_c, *value_c);
    }

    arena_destroy(allocator);
    return ret;
}

TestResult test_allocator_small_blocks() {
    CREATE_TESTRESULT()
    ArenaAllocator *allocator = arena_init();
    int const VARS_SIZE = 2048;
    int vars_reference[VARS_SIZE];
    int *vars[VARS_SIZE];

    for (int i = 0; i < VARS_SIZE; ++i) {
        vars_reference[i] = i;
        vars[i] = arena_allocate(allocator, sizeof(int));
        *vars[i] = i;
    }

    for (int i = 0; i < VARS_SIZE; ++i) {
        if (vars_reference[i] != *vars[i]) {
            ret.code = TestResultCode_FAILED_TEST;
            ret.message = calloc(1, 100);
            snprintf(ret.message, 99,
                     "Index [%d]: %d on stack and %d from allocator (address "
                     "%p) expected to be equal",
                     i, vars_reference[i], *vars[i], (void *)vars[i]);
            break;
        }
    }

    arena_destroy(allocator);
    return ret;
}

TestResult test_allocator_large_blocks() {
    CREATE_TESTRESULT()
    char const *const large_string =
        "Sed ut perspiciatis unde omnis iste natus error sit voluptatem "
        "accusantium doloremque laudantium, totam rem aperiam, eaque ipsa quae "
        "ab illo inventore veritatis et quasi architecto beatae vitae dicta "
        "sunt explicabo. Nemo enim ipsam voluptatem quia voluptas sit "
        "aspernatur aut odit aut fugit, sed quia consequuntur magni dolores "
        "eos qui ratione voluptatem sequi nesciunt. Neque porro quisquam est, "
        "qui dolorem ipsum quia dolor sit amet, consectetur, adipisci velit, "
        "sed quia non numquam eius modi tempora incidunt ut labore et dolore "
        "magnam aliquam quaerat voluptatem. Ut enim ad minima veniam, quis "
        "nostrum exercitationem ullam corporis suscipit laboriosam, nisi ut "
        "aliquid ex ea commodi consequatur? Quis autem vel eum iure "
        "reprehenderit qui in ea voluptate velit esse quam nihil molestiae "
        "consequatur, vel illum qui dolorem eum fugiat quo voluptas nulla "
        "pariatur? At vero eos et accusamus et iusto odio dignissimos ducimus "
        "qui blanditiis praesentium voluptatum deleniti atque corrupti quos "
        "dolores et quas molestias excepturi sint occaecati cupiditate non "
        "provident, similique sunt in culpa qui officia deserunt mollitia "
        "animi, id est laborum et dolorum fuga. Et harum quidem rerum facilis "
        "est et expedita distinctio. Nam libero tempore, cum soluta nobis est "
        "eligendi optio cumque nihil impedit quo minus id quod maxime placeat "
        "facere possimus, omnis voluptas assumenda est, omnis dolor "
        "repellendus. Temporibus autem quibusdam et aut officiis debitis aut "
        "rerum necessitatibus saepe eveniet ut et voluptates repudiandae sint "
        "et molestiae non recusandae. Itaque earum rerum hic tenetur a "
        "sapiente delectus, ut aut reiciendis voluptatibus maiores alias "
        "consequatur aut perferendis doloribus asperiores repellat.";

    if (strlen(large_string) < (size_t)AUTO_BLOCK_SIZE) {
        ret.code = TestResultCode_FAILED_PRECONDITION;
        ret.message =
            strdup("Test string is too small to test large block allocation.");
        return ret;
    }

    ArenaAllocator *allocator = arena_init();

    int *before = arena_allocate(allocator, sizeof(int));
    *before = -1;

    char const *const arena_string = arena_strdup(allocator, large_string);

    int *after = arena_allocate(allocator, sizeof(int));
    *after = -2;

    if (strcmp(large_string, arena_string)) {
        ret.code = TestResultCode_FAILED_TEST;
        ret.message = strdup(
            "large string arena allocation does not match original string.");
    }

    if (*after != -2 || *before != -1) {
        ret.code = TestResultCode_FAILED_TEST;
        ret.message = strdup(
            "Memory allocated before or after large block is corrupted.");
    }

    arena_destroy(allocator);
    return ret;
}
