#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_PATH_COUNT 256
#define MAX_PATH_LEN 256

typedef struct counter {
    unsigned long line_count;
    unsigned long word_count;
    unsigned long byte_count;
    char target[MAX_PATH_LEN];
} counter_t;

void print(counter_t *counter) {
    printf("%10lu %10lu %10lu %s\n", counter->line_count, counter->word_count, counter->byte_count, counter->target);
}

void wc(char *file, counter_t *counter) {

}

int main(int argc, char **argv) {
    int num = argc > 1 ? argc - 1 : 0;
    counter_t *counters = (counter_t *) malloc(num * sizeof(counter_t));

    if (counters != NULL) {
        counter_t total = {.line_count = 0, .word_count = 0, .byte_count = 0};
        strcpy(total.target, "total");

        for (int i = 1; i < argc; i++) {
            counter_t *curr_counter = counters + i - 1;
            wc(argv[i], curr_counter);
            print(curr_counter);
            total.line_count += curr_counter->line_count;
            total.word_count += curr_counter->word_count;
            total.byte_count += curr_counter->byte_count;
        }
        if (num > 1) print(&total);
    }

    free(counters);
    counters = NULL;
    return 0;
}