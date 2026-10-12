#include "stats.h"

/* largest of the first n values: the first value, or the largest of the rest */
float stats_max(const float a[], int n) {
    if (n == 1)
        return a[0];
    float rest = stats_max(a + 1, n - 1);
    return a[0] > rest ? a[0] : rest;
}


/* smallest of the first n values: start with a[0], keep anything smaller */
float stats_min(const float a[], int n) {

    float min = a[0];

    for (int i = 0; i < n; i++) {
        if (a[i] < min)
            min = a[i];
        
    }
    return min;
}

/* mean of the first n values: sum of the values divided by n */
float stats_mean(const float a[], int n){

    float sum = 0.0f;

    for (int i = 0; i < n; i++) {
        sum += a[i];
    }
    return sum / n;
}

/* longest run of consecutive values above the threshold: run counts the
   current run, best and best_start remember the longest one seen so far */
struct stats_longest_run_result stats_longest_run(const float a[], int n, float threshold) {

    struct stats_longest_run_result result;
    int run = 0, best = 0, best_start = -1, start = 0;

    for (int i = 0; i < n; i++) {
        if (a[i] > threshold) {
            if (run == 0)
                start = i;
            run++;
            if (run > best) {
                best = run;
                best_start = start;
            }
        } else {
            run = 0;
        }
    }

    result.length = best;
    result.start = best_start;
    return result;
}