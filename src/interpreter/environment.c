#include "environment.h"
#include <stdint.h>
#include <string.h>

static uint64_t hash(char const *const key, uint64_t modulo) {
    return 0;
    // TODO use a better hash than this shit thing

    // uint64_t sum = 0;
    // for (size_t i = 0; i < strlen(key); ++i) {
    //     sum += key[i];
    //     sum <<= 7;
    // }
    // return sum % modulo;
}

static bool place_in_map_is_empty(EnvironmentMapNode *place) {
    EnvironmentVariable variable = place->current;
    return variable.key == NULL;
}

static void place_in_map(EnvironmentMapNode *map, size_t map_size,
                         EnvironmentVariable variable) {
    uint64_t const hashed = hash(variable.key, map_size);
    EnvironmentMapNode *current_node = map + hashed;

    // if the immediate map entry is empty, fill that
    if (current_node->current.key == NULL) {
        current_node->current.key = strdup(variable.key);
        current_node->current.value = malloc(sizeof(Result));
        memcpy((void *restrict)current_node->current.value, variable.value,
               sizeof(Result));
        return;
    }

    while (current_node->next != NULL) {
        // update the current variable if an updated value is provided
        if (!strcmp(current_node->current.key, variable.key)) {
            free((void *)current_node->current.value);
            current_node->current.value = malloc(sizeof(Result));
            memcpy((void *restrict)current_node->current.value, variable.value,
                   sizeof(Result));
            return;
        }
        current_node = current_node->next;
    }

    // Create the new node to store the value upon collisions
    EnvironmentMapNode *new_node = calloc(1, sizeof(EnvironmentMapNode));
    new_node->current.key = strdup(variable.key);
    new_node->current.value = malloc(sizeof(*new_node->current.value));
    memcpy((void *restrict)new_node->current.value, variable.value,
           sizeof(*variable.value));
    current_node->next = new_node;
}

static void free_node_and_children(EnvironmentMapNode *node,
                                   bool free_kvpairs) {
    if (node->next != NULL) {
        if (free_kvpairs) {
            free((void *)node->current.key);
            free((void *)node->current.value);
        }
        free(node);
        return;
    }
    free_node_and_children(node->next, free_kvpairs);
}

static void free_map(EnvironmentMapNode *map, size_t map_size,
                     bool free_kvpairs) {
    for (size_t i = 0; i < map_size; ++i) {
        if (place_in_map_is_empty(map + i)) {
            continue;
        }
        free_node_and_children(map[i].next, free_kvpairs);
    }
}

static void double_map(Environment *env) {
    EnvironmentMapNode *doubled_map =
        calloc(env->map_size * 2, sizeof(EnvironmentMapNode));

    for (size_t i = 0; i < env->map_size; ++i) {
        if (place_in_map_is_empty(env->map + i)) {
            continue;
        }
        EnvironmentVariable variable = env->map[i].current;
        place_in_map(doubled_map, env->map_size * 2, variable);
    }

    // do not free the key/value memory since it is reused for the doubled map
    free_map(env->map, env->map_size, false);
    env->map_size *= 2;
    env->map = doubled_map;
}

/**
 * @brief Determine if the map storing variables in the environment needs
 * resizing.
 *
 * The map should be resized if it is more than half full of non-colliding keys.
 *
 * @param env The environment to examine.
 * @return True if the variable map should be resized, false otherwise.
 */
static bool needs_resizing(Environment const env) {
    size_t filled_spots = 0;
    for (size_t i = 0; i < env.map_size; ++i) {
        if (!place_in_map_is_empty(env.map + i)) {
            filled_spots++;
        }
    }
    return filled_spots > env.map_size / 2;
}

Result const *environment_read(Environment const env, char const *const key) {
    uint64_t const hashed = hash(key, env.map_size);
    EnvironmentMapNode current_node = env.map[hashed];

    while (current_node.next != NULL) {
        if (current_node.current.key != NULL &&
            !strcmp(current_node.current.key, key)) {
            break;
        }
        current_node = *current_node.next;
    }

    if (current_node.current.key != NULL &&
        strcmp(current_node.current.key, key)) {
        return NULL;
    }

    return current_node.current.value;
}

/**
 * @brief Define a variable in the given environment.
 *
 * This mallocs copies of variable.key and variable.value.
 *
 * @param env The environment to modify.
 * @param variable The variable to add.
 */
void environment_define(Environment *env, EnvironmentVariable variable) {
    if (needs_resizing(*env)) {
        double_map(env);
    }
    place_in_map(env->map, env->map_size, variable);
}

Environment *environment_init(size_t initial_size) {
    Environment *env = calloc(1, sizeof(Environment));
    env->map_size = initial_size;
    EnvironmentMapNode *map = calloc(initial_size, sizeof(EnvironmentMapNode));
    env->map = map;
    return env;
}

void environment_destroy(Environment *env) {
    free_map(env->map, env->map_size, true);
    free(env->map);
    free(env);
}
