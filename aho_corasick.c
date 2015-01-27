#include "aho_corasick.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

void stream_test(char* T, int n, char* P, int m, int* correct) {
    ac_state state = ac_build(P, m);
    ac_result result = ac_result_build();
    int j;
    for (j = 0; j < n; j++) {
        ac_stream(state, T[j], j, result);
        assert(correct[j] == result->j);
    }
    ac_free(state);
    ac_result_free(result);
}

int main(void) {
    int n = 18, m = 5;
    int correct_matches = 2;
    int* correct = malloc(correct_matches * sizeof(int));
    correct[0] = 8; correct[1] = 13;
    int* output = malloc(100 * sizeof(int));
    correct = realloc(correct, n * sizeof(int));
    correct[0]  = -1; correct[1]  = -1; correct[2]  = -1; correct[3]  = -1; correct[4]  = -1; correct[5]  = -1;
    correct[6]  = -1; correct[7]  = -1; correct[8]  = -1; correct[9]  = -1; correct[10] = -1; correct[11] = -1;
    correct[12] = 12; correct[13] = -1; correct[14] = -1; correct[15] = -1; correct[16] = -1; correct[17] = 17;
    stream_test("ababaabbababbababb", n, "ababb", m, correct);
    free(correct);
    free(output);
    printf("All tests succeeded!\n");
    return 0;
}