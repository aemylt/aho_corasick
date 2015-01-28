#ifndef __AHO_CORASICK__
#define __AHO_CORASICK__
#include <stdlib.h>
#include <string.h>

typedef struct trie_t {
    struct trie_t *child;
    char *child_keys;
    int children;
    struct trie_t *failure;
    char **output;
    int *m;
    int num_matches;
} *trie;

trie get_child(trie automata, char key) {
    int i;
    for (i = 0; i < automata->children; i++) {
        if (automata->child_keys[i] == key) return &(automata->child[i]);
    }
    return NULL;
}

void trie_free(trie automata) {
    int i;
    if (automata->children) {
        free(automata->child_keys);
        for (i = 0; i < automata->children; i++) trie_free(&automata->child[i]);
    }
    if (automata->num_matches) {
        for (i = 0; i < automata->num_matches; i++) {
            free(automata->output[i]);
        }
        free(automata->output);
        free(automata->m);
    }
    free(automata);
}

typedef struct ac_result_t {
    char **output;
    int *m;
    int num_matches;
    int j;
} *ac_result;

ac_result ac_result_build() {
    return malloc(sizeof(struct ac_result_t));
}

void ac_result_free(ac_result result) {
    free(result->output);
    free(result);
}

typedef struct ac_state_t {
    trie root;
    trie automata;
} *ac_state;

ac_state ac_build(char **P, int *m, int num_patterns) {
    int i, j;
    ac_state state = malloc(sizeof(struct ac_state_t));
    state->root = malloc(sizeof(struct trie_t));
    trie automata = state->root;
    trie has_child;
    for (j = 0; j < num_patterns; j++) {
        i = 0;
        while (i < m[j]) {
            has_child = get_child(automata, P[j][i]);
            if (has_child) {
                i++;
                automata = has_child;
            } else {
                break;
            }
        }
        while (i < m[j]) {
            automata->children += 1;
            automata->child_keys = realloc(automata->child_keys, sizeof(char) * automata->children);
            automata->child_keys[automata->children - 1] = P[j][i++];
            automata->child = realloc(automata->child, sizeof(struct trie_t) * automata->children);
            automata = &automata->child[automata->children - 1];
        }
        automata->num_matches = 1;
        automata->output = malloc(sizeof(char*));
        automata->output[0] = malloc(sizeof(char) * m[j]);
        strcpy(automata->output[0], P[j]);
        automata->m = malloc(sizeof(int));
        automata->m[0] = m[j];
        automata = state->root;
    }
    state->root->child->failure = state->root;
    trie failure = state->root->child->child;
    trie next;
    i = 1;
    while (!failure->num_matches) {
        next = get_child(automata, P[0][i]);
        while ((automata != state->root) && (!next)){ 
            automata = automata->failure;
            next = get_child(automata, P[0][i]);
        }
        if (next) automata = next;
        failure->failure = automata;
        failure = failure->child;
        i++;
    }
    failure->failure = automata;
    state->automata = state->root;
    return state;
}

void ac_stream(ac_state state, char T_j, int j, ac_result result) {
    result->j = -1;
    trie automata = state->automata;
    trie next = get_child(automata, T_j);
    while ((automata != state->root) && (!next)) {
        automata = automata->failure;
        next = get_child(automata, T_j);
    }
    if (next) automata = next;
    if (automata->num_matches) {
        result->j = j;
        result->num_matches = automata->num_matches;
        result->output = realloc(result->output, sizeof(char*) * result->num_matches);
        result->m = realloc(result->m, sizeof(int) * result->num_matches);
        int i;
        for (i = 0; i < result->num_matches; i++) {
            result->m[i] = automata->m[i];
            result->output[i] = realloc(result->output[i], sizeof(char) * result->m[i]);
            strcpy(result->output[i], automata->output[i]);
        }
        automata = automata->failure;
    }
    state->automata = automata;
}

void ac_free(ac_state state) {
    trie_free(state->root);
    free(state);
}

#endif
