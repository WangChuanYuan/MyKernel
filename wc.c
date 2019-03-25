#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define BUFFER_SIZE 1024
#define MAX_PATH_LEN 256

typedef struct counter {
    unsigned long line_count;
    unsigned long word_count;
    unsigned long byte_count;
    int is_dir;
    char target[MAX_PATH_LEN];
} counter_t;

void print(counter_t *counter) {
    if (counter->is_dir)
        printf("wc: %s: Is a directory\n", counter->target);
    printf("%6lu\t %6lu\t %6lu\t %s\n", counter->line_count, counter->word_count, counter->byte_count, counter->target);
}

int wc(char *file, counter_t *counter) {
    counter->line_count = 0;
    counter->word_count = 0;
    counter->byte_count = 0;
    strcpy(counter->target, file);

    struct stat info;
    if (stat(counter->target, &info) == -1)
        return 0;
    else if (S_ISDIR(info.st_mode)) {
        counter->is_dir = 1;
        return 1;
    }

    FILE *fp = fopen(file, "r");
    if (fp == NULL) return 0;
    char line[BUFFER_SIZE];
    while (fgets(line, BUFFER_SIZE, fp) != NULL) {
        int is_word_char = 0;
        for (int i = 0; i < strlen(line); i++) {
            char c = line[i];
            counter->byte_count++;
            if (c == '\n') {
                counter->line_count++;
                if (is_word_char) counter->word_count++;
                is_word_char = 0;
            } else if (c == ' ' || c == '\t' || c == '\v' || c == '\f' || c == '\r') {
                if (is_word_char) counter->word_count++;
                is_word_char = 0;
            } else is_word_char = 1;
        }
        if (is_word_char) counter->word_count++;
    }
    return 1;
}

int main(int argc, char **argv) {
    int num = argc > 1 ? argc - 1 : 0;
    counter_t *counters = (counter_t *) malloc(num * sizeof(counter_t));

    if (counters != NULL) {
        counter_t total = {.line_count = 0, .word_count = 0, .byte_count = 0, .is_dir = 0};
        strcpy(total.target, "total");

        for (int i = 1; i < argc; i++) {
            counter_t *curr_counter = counters + i - 1;
            if (wc(argv[i], curr_counter))
                print(curr_counter);
            else
                fprintf(stderr, "wc: cannot open '%s'\n", argv[i]);
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