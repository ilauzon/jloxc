#pragma once

#include <string.h>

#define TESTRESULTCODE_ENUM_LIST(X)                                            \
    X(OK)                                                                      \
    X(FAILED_TEST)                                                             \
    X(FAILED_PRECONDITION)

enum TestResultCode {
#define GENERATE_ENUM(name) TestResultCode##_##name,
    TESTRESULTCODE_ENUM_LIST(GENERATE_ENUM)
#undef GENERATE_ENUM
};

static char const *testresultcode_to_string(enum TestResultCode t) {
    switch (t) {
#define GENERATE_SWITCH_CASE(name)                                             \
    case TestResultCode##_##name:                                              \
        return #name;
        TESTRESULTCODE_ENUM_LIST(GENERATE_SWITCH_CASE)
#undef GENERATE_SWITCH_CASE
    }
    return NULL;
}

typedef struct {
    char const *name;
    enum TestResultCode code;
    char *message;
} TestResult;

typedef TestResult (*TestFunction)(void);

TestResult test_allocator_allocate();
TestResult test_allocator_small_blocks();
TestResult test_allocator_large_blocks();

static TestFunction tests[] = {
    test_allocator_allocate,
    test_allocator_small_blocks,
    test_allocator_large_blocks,
};

#define CREATE_TESTRESULT()                                                    \
    TestResult ret = {0};                                                      \
    ret.name = __func__;
