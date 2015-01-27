#ifndef __AHO_CORASICK__
#define __AHO_CORASICK__
#include <stdlib.h>
#include <string.h>

typedef struct trie_t {
    struct trie_t *child;
    struct trie_t *failure;
    char P;
    int end;
    char *output;
    int m;
} *trie;

void trie_free(trie automata) {
    if (!automata->end) trie_free(automata->child);
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

ac_state ac_build(char* P, int m) {
    int i;
    ac_state state = malloc(sizeof(struct ac_state_t));
    state->root = malloc(sizeof(struct trie_t));
    state->root->child = malloc(sizeof(struct trie_t));
    trie automata = state->root->child;
    for (i = 0; i < m; i++) {
        automata->P = P[i];
        automata->end = (i == m - 1);
        if (!automata->end) {
            automata->child = malloc(sizeof(struct trie_t));
            automata = automata->child;
        } else {
            automata->output = malloc(sizeof(char) * m);
            strcpy(automata->output, P);
            automata->m = m;
        }
    }
    automata = state->root;
    state->root->child->failure = state->root;
    trie failure = state->root->child->child;
    while (!failure->end) {
        while ((automata != state->root) && (failure->P != automata->child->P)) automata = automata->failure;
        if (failure->P == automata->child->P) automata = automata->child;
        failure->failure = automata;
        failure = failure->child;
    }
    while ((automata != state->root) && (failure->P != automata->child->P)) automata = automata->failure;
    if (failure->P == automata->child->P) automata = automata->child;
    failure->failure = automata;
    state->automata = state->root;
    return state;
}

void ac_stream(ac_state state, char T_j, int j, ac_result result) {
    result->j = -1;
    trie automata = state->automata;
    while ((automata != state->root) && (automata->child->P != T_j)) automata = automata->failure;
    if (automata->child->P == T_j) automata = automata->child;
    if (automata->end) {
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
