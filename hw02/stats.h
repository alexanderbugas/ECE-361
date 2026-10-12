#ifndef STATS_H
#define STATS_H

/* what stats_longest_run gives back */
struct stats_longest_run_result {
    int length;     /* number of values in the longest run, 0 if there is none */
    int start;      /* index of the first value in that run, -1 if there is none */
};

/*
 * stats_max: the largest value in an array, found recursively.
 *
 * Takes:   a: the values (any float array, e.g. temperatures or
 *             humidities); it is not changed
 *          n: how many values to look at, starting at index 0
 * Returns: the largest of a[0] .. a[n-1]
 * Assumes: n >= 1. With n == 0 the base case is never reached.
 *          Each call uses one stack frame, so very large n can overflow the
 *          stack.
 */
float stats_max(const float a[], int n);

/*
 * stats_min: the smallest value in an array.
 *
 * Takes:   a: the values; it is not changed
 *          n: how many values to look at, starting at index 0
 * Returns: the smallest of a[0] .. a[n-1]
 * Assumes: n >= 1. It starts from a[0], which does not exist when n == 0.
 */
float stats_min(const float a[], int n);

/*
 * stats_mean: the average of the values in an array.
 *
 * Takes:   a: the values; it is not changed
 *          n: how many values to average, starting at index 0
 * Returns: (a[0] + ... + a[n-1]) / n, summed as a float in index order
 * Assumes: n >= 1. With n == 0 it divides by zero and returns NaN.
 */
float stats_mean(const float a[], int n);

/*
 * stats_longest_run: the longest stretch of consecutive values strictly
 * above a threshold. A value equal to the threshold is not above it, so it
 * ends a run.
 *
 * Takes:   a: the values; it is not changed
 *          n: how many values to look at, starting at index 0
 *          threshold: a value must be greater than this to count
 * Returns: the run's length and the index where it starts. If two runs are
 *          equally long, the first one. If no value is above the threshold,
 *          length 0 and start -1 (start must not be used as an index then).
 * Assumes: n >= 0.
 */
struct stats_longest_run_result stats_longest_run(const float a[], int n, float threshold);

#endif /* STATS_H */
