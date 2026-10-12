/*
 * main.c: ECE 361, HW2
 *
 * Reads sensor readings from standard input and prints a summary: the number
 * of readings, min/max/mean temperature and humidity, the longest run above a
 * temperature threshold, and a histogram of the temperatures.
 *
 * Usage: ./readings [threshold] < data/normal.txt
 *        threshold is in degrees C, default 30.0
 *
 * main owns the data and does the printing. The reading, stats, and histogram
 * modules only receive the arrays they need as parameters.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>     /* not used yet; kept for later work */
#include "reading.h"
#include "stats.h"
#include "histogram.h"

int main(int argc, char *argv[]) {
    int   ticks[MAX_READINGS];
    float temps[MAX_READINGS];
    float hums[MAX_READINGS];
    float threshold = 30.0f;

    /* command line: at most one argument, the threshold */
    if (argc > 2) {
        fprintf(stderr, "usage: %s [threshold] < readings.txt\n", argv[0]);
        return 1;
    }
    if (argc == 2) {
        char *end;
        threshold = strtof(argv[1], &end);
        if (*end != '\0') {
            fprintf(stderr, "error: threshold '%s' is not a number\n", argv[1]);
            return 1;
        }
    }

    /* fill the three arrays; r holds how many readings and skipped lines */
    struct reading_result r = reading_read(ticks, temps, hums, MAX_READINGS);

    printf("readings: %d\n", r.count);
    printf("skipped:  %d\n", r.skipped);
    if (r.count == 0) {
        printf("no readings, no summary\n");
        return 0;
    }
    /* from here on r.count >= 1, which the stats functions assume */

    /* one function per statistic, called once for each array */
    printf("temperature: min %6.1f  max %6.1f  mean %6.2f C\n", stats_min(temps, r.count), stats_max(temps, r.count), stats_mean(temps, r.count));
    printf("humidity:    min %6.1f  max %6.1f  mean %6.2f %%RH\n", stats_min(hums, r.count), stats_max(hums, r.count), stats_mean(hums, r.count));

    /* stats returns an index; main turns it into tick numbers with ticks[] */
    struct stats_longest_run_result longest_run = stats_longest_run(temps, r.count, threshold);

    if (longest_run.length == 0)
        printf("above %.1f C: never\n", threshold);
    else
        printf("above %.1f C: longest run %d readings, from tick %d to tick %d\n",
               threshold, longest_run.length, ticks[longest_run.start], ticks[longest_run.start + longest_run.length - 1]);
    histogram_print(temps, r.count);
    return 0;
}
