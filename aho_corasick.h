#ifndef __AHO_CORASICK__
#define __AHO_CORASICK__
#include <stdlib.h>
#include "hash_lookup.h"

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

typedef struct ac_state_t {
    int num_nodes;
    int *has_match;
    hash_lookup *lookup;
    int state;
} *ac_state;

int get_child(int *num_children, int **children, char **child_keys, int state, char key) {
    int i;
    for (i = 0; i < num_children[state]; i++) {
        if (child_keys[state][i] == key) return children[state][i];
    }
    return -1;
}

ac_state ac_build(char **P, int *m, int num_patterns) {
    ac_state build = malloc(sizeof(struct ac_state_t));
    build->state = 0;
    int num_nodes = 1;
    int *num_children = malloc(sizeof(int));
    num_children[0] = 0;
    int **children = malloc(sizeof(int*));
    children[0] = NULL;
    char **child_keys = malloc(sizeof(char*));
    child_keys[0] = NULL;
    build->has_match = malloc(sizeof(int*));
    build->has_match[0] = 0;

    int i, j, state, next;
    for (i = 0; i < num_patterns; i++) {
        state = 0;
        j = 0;
        next = get_child(num_children, children, child_keys, state, P[i][j]);
        while ((j < m[i]) && (next != -1)) {
            state = next;
            next = get_child(num_children, children, child_keys, state, P[i][++j]);
        }
        for (; j < m[i]; j++) {
            num_nodes++;
            num_children = realloc(num_children, sizeof(int) * num_nodes);
            num_children[num_nodes - 1] = 0;
            children = realloc(children, sizeof(int*) * num_nodes);
            children[num_nodes - 1] = NULL;
            child_keys = realloc(child_keys, sizeof(char*) * num_nodes);
            child_keys[num_nodes - 1] = NULL;
            build->has_match = realloc(build->has_match, sizeof(int*) * num_nodes);
            build->has_match[num_nodes - 1] = 0;

            num_children[state]++;
            children[state] = realloc(children[state], sizeof(int) * num_children[state]);
            children[state][num_children[state] - 1] = num_nodes - 1;
            child_keys[state] = realloc(child_keys[state], sizeof(char) * num_children[state]);
            child_keys[state][num_children[state] - 1] = P[i][j];
            state = num_nodes - 1;
        }
        build->has_match[state] = 1;
    }
    build->num_nodes = num_nodes;

    int *failure = malloc(sizeof(int) * num_nodes);
    state_queue queue = queue_build();
    for (i = 0; i < num_children[0]; i++) {
        push(queue, children[0][i]);
        failure[children[0][i]] = 0;
    }
    while (queue->size) {
        i = pop(queue);
        for (j = 0; j < num_children[i]; j++) {
            push(queue, children[i][j]);
            state = failure[i];
            next = get_child(num_children, children, child_keys, state, child_keys[i][j]);
            while ((state) && (next == -1)) {
                state = failure[state];
                next = get_child(num_children, children, child_keys, state, child_keys[i][j]);
            }
            if (next != -1) state = next;
            failure[children[i][j]] = state;
            build->has_match[children[i][j]] |= build->has_match[failure[children[i][j]]];
        }
    }

    int k, add;
    int *num_moves = malloc(sizeof(int) * num_nodes);
    int **next_move = malloc(sizeof(int*) * num_nodes);
    char **move_keys = malloc(sizeof(char*) * num_nodes);
    num_moves[0] = num_children[0];
    next_move[0] = malloc(sizeof(int) * num_children[0]);
    move_keys[0] = malloc(sizeof(char) * num_children[0]);
    for (i = 0; i < num_children[0]; i++) {
        push(queue, children[0][i]);
        next_move[0][i] = children[0][i];
        move_keys[0][i] = child_keys[0][i];
    }
    while (queue->size) {
        i = pop(queue);
        num_moves[i] = num_children[i];
        next_move[i] = malloc(sizeof(int) * num_children[i]);
        move_keys[i] = malloc(sizeof(char) * num_children[i]);
        for (j = 0; j < num_children[i]; j++) {
            push(queue, children[i][j]);
            next_move[i][j] = children[i][j];
            move_keys[i][j] = child_keys[i][j];
        }
        state = failure[i];
        for (j = 0; j < num_moves[state]; j++) {
            add = 1;
            for (k = 0; k < num_children[i]; k++) {
                if (child_keys[i][k] == move_keys[state][j]) {
                    add = 0;
                    break;
                }
            }
            if (add) {
                num_moves[i]++;
                next_move[i] = realloc(next_move[i], sizeof(int) * num_moves[i]);
                next_move[i][num_moves[i] - 1] = next_move[state][j];
                move_keys[i] = realloc(move_keys[i], sizeof(char) * num_moves[i]);
                move_keys[i][num_moves[i] - 1] = move_keys[state][j];
            }
        }
    }
    queue_free(queue);

    build->lookup = malloc(sizeof(hash_lookup) * num_nodes);
    free(failure);
    for (i = 0; i < num_nodes; i++) {
        build->lookup[i] = hashlookup_build(move_keys[i], next_move[i], num_moves[i]);
        if (num_children[i]) {
            free(children[i]);
            free(child_keys[i]);
        }
        if (num_moves[i]) {
            free(move_keys[i]);
            free(next_move[i]);
        }
    }
    free(num_children);
    free(children);
    free(child_keys);
    free(num_moves);
    free(move_keys);
    free(next_move);

    return build;
}

int ac_stream(ac_state automata, char T_j, int j) {
    int result = -1;
    int state = hashlookup_search(automata->lookup[automata->state], T_j);
    if (automata->has_match[state]) {
        result = j;
    }
    automata->state = state;
    return result;
}

void ac_free(ac_state state) {
    int i;
    for (i = 0; i < state->num_nodes; i++) {
        hashlookup_free(&state->lookup[i]);
    }
    free(state->lookup);
    free(state->has_match);
    free(state);
}

#endif
