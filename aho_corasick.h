#ifndef __AHO_CORASICK__
#define __AHO_CORASICK__
#include <stdlib.h>

typedef struct ac_state_t {
    char* P;
    int m;
    int i;
    int* failure;
} *ac_state;

ac_state ac_build(char* P, int m) {
    int i, j;
    ac_state state = malloc(sizeof(struct ac_state_t));
    state->P = malloc(m * sizeof(char));
    for (i = 0; i < m; i++) state->P[i] = P[i];
    state->m = m;
    state->i = -1;
    state->failure = malloc(m * sizeof(int));
    i = -1;
    state->failure[0] = -1;
    for (j = 1; j < m; j++) {
        while (i > -1 && P[i + 1] != P[j]) i = state->failure[i];
        if (P[i + 1] == P[j]) i++;
        state->failure[j] = i;
    }
    return state;
}

int ac_stream(ac_state state, char T_j, int j) {
    char* P = state->P;
    int i = state->i, result = -1;
    int* failure = state->failure;
    while (i > -1 && P[i + 1] != T_j) i = failure[i];
    if (P[i + 1] == T_j) i++;
    if (i == state->m - 1) {
        result = j;
        i = failure[i];
    }
    state->i = i;
    return result;
}

void ac_free(ac_state state) {
    free(state->failure);
    free(state->P);
    free(state);
}

#endif
