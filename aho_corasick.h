#ifndef __AHO_CORASICK__
#define __AHO_CORASICK__
#include <stdlib.h>

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
    char ***matches;
    int **length;
    int *num_matches;
    int *num_moves;
    int **next_move;
    char **move_keys;
    int state;
} *ac_state;

int get_child(int *num_children, int **children, char **child_keys, int state, char key) {
    int i;
    for (i = 0; i < num_children[state]; i++) {
        if (child_keys[state][i] == key) return children[state][i];
    }
    return -1;
}

int next_move(ac_state automata, int state, char key) {
    int i;
    for (i = 0; i < automata->num_moves[state]; i++) {
        if (automata->move_keys[state][i] == key) return automata->next_move[state][i];
    }
    return 0;
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
            build->num_matches = realloc(build->num_matches, sizeof(int*) * num_nodes);
            build->num_matches[num_nodes - 1] = 0;
            build->matches = realloc(build->matches, sizeof(char**) * num_nodes);
            build->matches[num_nodes - 1] = NULL;
            build->length = realloc(build->length, sizeof(int*) * num_nodes);
            build->length[num_nodes - 1] = NULL;

            num_children[state]++;
            children[state] = realloc(children[state], sizeof(int) * num_children[state]);
            children[state][num_children[state] - 1] = num_nodes - 1;
            child_keys[state] = realloc(child_keys[state], sizeof(char) * num_children[state]);
            child_keys[state][num_children[state] - 1] = P[i][j];
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
        }
    }

    int k, add;
    build->num_moves = malloc(sizeof(int) * num_nodes);
    build->next_move = malloc(sizeof(int*) * num_nodes);
    build->move_keys = malloc(sizeof(char*) * num_nodes);
    build->num_moves[0] = num_children[0];
    build->next_move[0] = malloc(sizeof(int) * num_children[0]);
    build->move_keys[0] = malloc(sizeof(char) * num_children[0]);
    for (i = 0; i < num_children[0]; i++) {
        push(queue, children[0][i]);
        build->next_move[0][i] = children[0][i];
        build->move_keys[0][i] = child_keys[0][i];
    }
    while (queue->size) {
        i = pop(queue);
        build->num_moves[i] = num_children[i];
        build->next_move[i] = malloc(sizeof(int) * num_children[i]);
        build->move_keys[i] = malloc(sizeof(char) * num_children[i]);
        for (j = 0; j < num_children[i]; j++) {
            push(queue, children[i][j]);
            build->next_move[i][j] = children[i][j];
            build->move_keys[i][j] = child_keys[i][j];
        }
        state = failure[i];
        for (j = 0; j < build->num_moves[state]; j++) {
            add = 1;
            for (k = 0; k < num_children[i]; k++) {
                if (child_keys[i][k] == build->move_keys[state][j]) {
                    add = 0;
                    break;
                }
            }
            if (add) {
                build->num_moves[i]++;
                build->next_move[i] = realloc(build->next_move[i], sizeof(int) * build->num_moves[i]);
                build->next_move[i][build->num_moves[i] - 1] = build->next_move[state][j];
                build->move_keys[i] = realloc(build->move_keys[i], sizeof(char) * build->num_moves[i]);
                build->move_keys[i][build->num_moves[i] - 1] = build->move_keys[state][j];
            }
        }
    }
    queue_free(queue);

    free(failure);
    for (i = 0; i < num_nodes; i++) {
        if (num_children[i]) {
            free(children[i]);
            free(child_keys[i]);
        }
    }
    free(num_children);
    free(children);
    free(child_keys);

    return build;
}

void ac_stream(ac_state automata, char T_j, int j, ac_result result) {
    result->j = -1;
    int state = next_move(automata, automata->state, T_j);
    if (automata->num_matches[state]) {
        result->j = j;
        result->num_matches = automata->num_matches[state];
        result->output = automata->matches[state];
        result->m = automata->length[state];
    }
    automata->state = state;
}

void ac_free(ac_state state) {
    int i, j;
    for (i = 0; i < state->num_nodes; i++) {
        if (state->num_moves[i]) {
            free(state->next_move[i]);
            free(state->move_keys[i]);
        }
        for (j = 0; j < state->num_matches[i]; j++) {
            free(state->matches[i][j]);
        }
        if (state->num_matches[i]) {
            free(state->matches[i]);
            free(state->length[i]);
        }
    }
    free(state->num_matches);
    free(state->matches);
    free(state->num_moves);
    free(state->next_move);
    free(state->move_keys);
    free(state);
}

#endif
