#include "reading.h"
#include <stdio.h>

#define LINE_LEN     128

/* 1 if the line is blank or a comment (after leading spaces and tabs), 0 if not */
static int is_blank_or_comment(const char line[]) {
    int i = 0;
    while (line[i] == ' ' || line[i] == '\t')
        i++;
    return line[i] == '\n' || line[i] == '\0' || line[i] == '#';
}

/* read lines until end of input or max readings; see reading.h */
struct reading_result reading_read(int ticks[], float temps[], float hums[], int max) {
    struct reading_result r = {0, 0};
    char line[LINE_LEN];

    while (fgets(line, sizeof line, stdin) != NULL) {
        if (is_blank_or_comment(line))
            continue;
        if (r.count == max) {
            fprintf(stderr, "warning: more than %d readings, the rest are ignored\n", max);
            break;
        }
        if (sscanf(line, "%d %f %f", &ticks[r.count], &temps[r.count], &hums[r.count]) == 3)
            r.count++;
        else
            r.skipped++;
    }
    return r;
}
