#ifndef __AHO_CORASICK__
#define __AHO_CORASICK__
#include <stdlib.h>

#include <stdio.h>

typedef struct state_queue_item_t {
    struct state_queue_item_t *next;
    int item;
} *state_queue_item;

typedef struct state_queue_t {
    int size;
    state_queue_item start;
    state_queue_item end;
} *state_queue;

state_queue queue_build() {
    state_queue queue = malloc(sizeof(struct state_queue_t));
    queue->size = 0;
    return queue;
}

void push(state_queue queue, int item) {
    if (queue->size) {
        queue->end->next = malloc(sizeof(struct state_queue_item_t));
        queue->end->next->item = item;
        queue->end = queue->end->next;
    } else {
        queue->start = malloc(sizeof(struct state_queue_item_t));
        queue->start->item = item;
        queue->end = queue->start;
    }
    queue->size++;
}

int pop(state_queue queue) {
    queue->size--;
    state_queue_item start = queue->start;
    int item = start->item;
    queue->start = start->next;
    free(start);
    return item;
}

void queue_free(state_queue queue) {
    free(queue);
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
    free(result);
}

typedef struct ac_state_t {
    int num_nodes;
    int *num_children;
    int **children;
    char **child_keys;
    char ***matches;
    int **length;
    int *num_matches;
    int *failure;
    int state;
} *ac_state;

int get_child(ac_state automata, int state, char key) {
    int i;
    for (i = 0; i < automata->num_children[state]; i++) {
        if (automata->child_keys[state][i] == key) return automata->children[state][i];
    }
    return -1;
}

ac_state ac_build(char **P, int *m, int num_patterns) {
    ac_state build = malloc(sizeof(struct ac_state_t));
    build->state = 0;
    int num_nodes = 1;
    build->num_children = malloc(sizeof(int));
    build->num_children[0] = 0;
    build->children = malloc(sizeof(int*));
    build->children[0] = NULL;
    build->child_keys = malloc(sizeof(char*));
    build->child_keys[0] = NULL;
    build->num_matches = malloc(sizeof(int*));
    build->num_matches[0] = 0;
    build->matches = malloc(sizeof(char**));
    build->matches[0] = NULL;
    build->length = malloc(sizeof(int*));
    build->length[0] = NULL;

    int i, j, state, next;
    for (i = 0; i < num_patterns; i++) {
        state = 0;
        j = 0;
        next = get_child(build, state, P[i][j]);
        while ((j < m[i]) && (next != -1)) {
            state = next;
            next = get_child(build, state, P[i][++j]);
        }
        for (; j < m[i]; j++) {
            num_nodes++;
            build->num_children = realloc(build->num_children, sizeof(int) * num_nodes);
            build->num_children[num_nodes - 1] = 0;
            build->children = realloc(build->children, sizeof(int*) * num_nodes);
            build->children[num_nodes - 1] = NULL;
            build->child_keys = realloc(build->child_keys, sizeof(char*) * num_nodes);
            build->child_keys[num_nodes - 1] = NULL;
            build->num_matches = realloc(build->num_matches, sizeof(int*) * num_nodes);
            build->num_matches[num_nodes - 1] = 0;
            build->matches = realloc(build->matches, sizeof(char**) * num_nodes);
            build->matches[num_nodes - 1] = NULL;
            build->length = realloc(build->length, sizeof(int*) * num_nodes);
            build->length[num_nodes - 1] = NULL;

            build->num_children[state]++;
            build->children[state] = realloc(build->children[state], sizeof(int) * build->num_children[state]);
            build->children[state][build->num_children[state] - 1] = num_nodes - 1;
            build->child_keys[state] = realloc(build->child_keys[state], sizeof(char) * build->num_children[state]);
            build->child_keys[state][build->num_children[state] - 1] = P[i][j];
            state = num_nodes - 1;
        }
        build->num_matches[state] = 1;
        build->length[state] = malloc(sizeof(int));
        build->length[state][0] = m[i];
        build->matches[state] = malloc(sizeof(char*));
        build->matches[state][0] = malloc(sizeof(char) * m[i]);
        for (j = 0; j < m[i]; j++) {
            build->matches[state][0][j] = P[i][j];
        }
    }

    build->failure = malloc(sizeof(int) * num_nodes);
    state_queue queue = queue_build();
    for (i = 0; i < build->num_children[0]; i++) {
        push(queue, build->children[0][i]);
        build->failure[build->children[0][i]] = 0;
    }
    while (queue->size) {
        i = pop(queue);
        for (j = 0; j < build->num_children[i]; j++) {
            push(queue, build->children[i][j]);
            state = build->failure[i];
            next = get_child(build, state, build->child_keys[i][j]);
            while ((state) && (next == -1)) {
                state = build->failure[state];
                next = get_child(build, state, build->child_keys[i][j]);
            }
            if (next != -1) state = next;
            build->failure[build->children[i][j]] = state;
        }
    }
    queue_free(queue);
    build->num_nodes = num_nodes;
    return build;
}

void ac_stream(ac_state automata, char T_j, int j, ac_result result) {
    result->j = -1;
    int state = automata->state;
    int next = get_child(automata, state, T_j);
    while ((state) && (next == -1)) {
        state = automata->failure[state];
        next = get_child(automata, state, T_j);
    }
    if (next != -1) state = next;
    if (automata->num_matches[state]) {
        result->j = j;
        result->num_matches = automata->num_matches[state];
        result->output = automata->matches[state];
        result->m = automata->length[state];
    }
    automata->state = state;
}

void ac_free(ac_state state) {
    free(state->failure);
    int i, j;
    for (i = 0; i < state->num_nodes; i++) {
        if (state->num_children[i]) {
            free(state->children[i]);
            free(state->child_keys[i]);
        }
        for (j = 0; j < state->num_matches[i]; j++) {
            free(state->matches[i][j]);
        }
        if (state->num_matches[i]) {
            free(state->matches[i]);
            free(state->length[i]);
        }
    }
    free(state->num_children);
    free(state->num_matches);
    free(state->matches);
    free(state->children);
    free(state->child_keys);
    free(state);
}

#endif
