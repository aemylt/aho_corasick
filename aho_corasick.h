#ifndef __AHO_CORASICK__
#define __AHO_CORASICK__
#include <stdlib.h>
#include <string.h>

typedef struct trie_t {
    struct trie_t *child;
    char *child_keys;
    int children;
    struct trie_t *failure;
    char *output;
    int m;
} *trie;

trie get_child(trie automata, char key) {
    int i;
    for (i = 0; i < automata->children; i++) {
        if (automata->child_keys[i] == key) return &(automata->child[i]);
    }
    return NULL;
}

void trie_free(trie automata) {
    if (!automata->m) {
        free(automata->child_keys);
        trie_free(automata->child);
    }
    else free(automata->output);
    free(automata);
}

typedef struct ac_result_t {
    char *output;
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

ac_state ac_build(char *P, int m) {
    int i;
    ac_state state = malloc(sizeof(struct ac_state_t));
    state->root = malloc(sizeof(struct trie_t));
    state->root->child = malloc(sizeof(struct trie_t));
    state->root->child_keys = malloc(sizeof(char));
    state->root->child_keys[0] = P[0];
    state->root->children = 1;
    trie automata = state->root->child;
    for (i = 1; i < m; i++) {
        automata->child_keys = malloc(sizeof(char));
        automata->child_keys[0] = P[i];
        automata->children = 1;
        automata->m = 0;
        automata->child = malloc(sizeof(struct trie_t));
        automata = automata->child;
    }
    automata->output = malloc(sizeof(char) * m);
    strcpy(automata->output, P);
    automata->m = m;
    automata = state->root;
    state->root->child->failure = state->root;
    trie failure = state->root->child->child;
    trie next;
    i = 1;
    while (!failure->m) {
        next = get_child(automata, P[i]);
        while ((automata != state->root) && (!next)){ 
            automata = automata->failure;
            next = get_child(automata, P[i]);
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
    if (automata->m) {
        result->j = j;
        result->output = realloc(result->output, sizeof(char) * automata->m);
        strcpy(result->output, automata->output);
        automata = automata->failure;
    }
    state->automata = automata;
}

void ac_free(ac_state state) {
    trie_free(state->root);
    free(state);
}

#endif
