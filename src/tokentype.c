#include "tokentype.h"
#include "stdbool.h"
#include <stdlib.h>
#include <string.h>

#define STRING_HASHMAP_SIZE 50

typedef struct StringHashmapNode {
    char *key;
    enum TokenType value;
    struct StringHashmapNode *next;
} StringHashmapNode;

typedef struct {
    StringHashmapNode members[STRING_HASHMAP_SIZE];
    bool present[STRING_HASHMAP_SIZE];
} StringHashmap;

static StringHashmap keywords = {0};
static bool keywords_set = false;

/**
 * @brief djb2 hash algorithm
 *
 * @param key The key to hash.
 * @return The hashed value.
 */
unsigned int tokentype_hash(char const *const key) {
    unsigned long hash = 5381;
    int c;
    int i = 0;
    while ((c = *(key + i)))
        i++;
    hash = ((hash << 5) + hash) + c; // hash * 33 + c
    return hash % STRING_HASHMAP_SIZE;
}

/**
 * @brief Get a value from a hashmap.
 *
 * @param hashmap The hashmap to search in.
 * @param key The search key.
 * @param value Where the value will be returned. Not modified if the value is
 * not found.
 * @return True if the value was found, false if not.
 */
bool tokentype_stringhashmap_get(StringHashmap const *const hashmap,
                                 char const *const key, enum TokenType *value) {
    int const hashed = tokentype_hash(key);
    StringHashmapNode const *node = hashmap->members + hashed;
    bool present = hashmap->present[hashed];
    if (!present) {
        return false;
    }
    while (node) {
        if (0 == strcmp(node->key, key)) {
            *value = node->value;
            return true;
        }
        node = node->next;
    }
    return false;
}

/**
 * @brief Set a key-value pair in the hashmap.
 *
 * @param hashmap The hashmap to modify.
 * @param key The key to set.
 * @param value The value to set.
 */
void tokentype_stringhashmap_set(StringHashmap *const hashmap, char *const key,
                                 enum TokenType const value) {
    int const hashed = tokentype_hash(key);
    StringHashmapNode *node = hashmap->members + hashed;
    bool present = hashmap->present[hashed];
    StringHashmapNode new_node = {
        .key = key,
        .value = value,
        .next = NULL,
    };
    if (!present) {
        memcpy(node, &new_node, sizeof(StringHashmapNode));
        hashmap->present[hashed] = true;
        return;
    }

    while (node) {
        if (strcmp(node->key, key) == 0) {
            hashmap->present[hashed] = true;
            return;
        }
        if (!(node->next)) {
            StringHashmapNode *new_node_location =
                malloc(sizeof(StringHashmapNode));
            node->next = new_node_location;
            memcpy(node->next, &new_node, sizeof(StringHashmapNode));
            hashmap->present[hashed] = true;
            return;
        }
        node = node->next;
    }
}

void tokentype_ensure_keywords_set(void) {
    if (keywords_set) {
        return;
    }
    keywords_set = true;
    tokentype_stringhashmap_set(&keywords, "and", TokenType_AND);
    tokentype_stringhashmap_set(&keywords, "class", TokenType_CLASS);
    tokentype_stringhashmap_set(&keywords, "else", TokenType_ELSE);
    tokentype_stringhashmap_set(&keywords, "false", TokenType_FALSE);
    tokentype_stringhashmap_set(&keywords, "for", TokenType_FOR);
    tokentype_stringhashmap_set(&keywords, "fun", TokenType_FUN);
    tokentype_stringhashmap_set(&keywords, "if", TokenType_IF);
    tokentype_stringhashmap_set(&keywords, "nil", TokenType_NIL);
    tokentype_stringhashmap_set(&keywords, "or", TokenType_OR);
    tokentype_stringhashmap_set(&keywords, "print", TokenType_PRINT);
    tokentype_stringhashmap_set(&keywords, "return", TokenType_RETURN);
    tokentype_stringhashmap_set(&keywords, "super", TokenType_SUPER);
    tokentype_stringhashmap_set(&keywords, "this", TokenType_THIS);
    tokentype_stringhashmap_set(&keywords, "true", TokenType_TRUE);
    tokentype_stringhashmap_set(&keywords, "var", TokenType_VAR);
    tokentype_stringhashmap_set(&keywords, "while", TokenType_WHILE);
}

char const *tokentype_to_string(enum TokenType t) {
    switch (t) {
#define GENERATE_SWITCH_CASE(name)                                             \
    case TokenType##_##name:                                                   \
        return "TokenType_" #name;
        TOKENTYPE_ENUM_LIST(GENERATE_SWITCH_CASE)
#undef GENERATE_SWITCH_CASE
    }
    return NULL;
}

enum TokenType tokentype_from_string(char const *const string) {
    tokentype_ensure_keywords_set();
    enum TokenType value = 0;
    bool found = tokentype_stringhashmap_get(&keywords, string, &value);
    if (!found) {
        return TokenType_IDENTIFIER;
    }
    return value;
}
