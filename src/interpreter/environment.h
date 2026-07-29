#pragma once
#include "result.h"
#include <stddef.h>
#include <stdlib.h>

typedef struct {
    char const *key;
    Result const *value;
} EnvironmentVariable;

typedef struct EnvironmentMapNode {
    EnvironmentVariable current;
    struct EnvironmentMapNode *next;
} EnvironmentMapNode;

typedef struct {
    EnvironmentMapNode *map;
    size_t map_size;
} Environment;

void environment_define(Environment *env, EnvironmentVariable variable);

Result const *environment_read(Environment const env, char const *const key);

Environment *environment_init(size_t initial_size);
