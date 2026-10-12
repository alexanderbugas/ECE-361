/*
 * test_stats.c: unit tests for the stats module, ECE 361 HW2
 *
 * Prints PASS or FAIL for each test and a summary.
 * Exits with 1 if any test fails, so that make test fails.
 *
 * Most tests sit on an edge (the smallest n, the first and last index, a value
 * exactly at the threshold), because that is where off-by-one bugs show up.
 * Each test uses a small array whose answer is worked out by hand.
 *
 * Comparing floats with == is usually risky because of rounding. It is safe
 * here: stats_max and stats_min return one of the array's own values
 * unchanged, and the means tested divide exactly.
 */
#include <stdio.h>
#include "stats.h"

static int passed = 0;
static int failed = 0;

/* record one result: cond is nonzero if the test passed */
static void check(int cond, const char *name) {
    if (cond) {
        printf("PASS  %s\n", name);
        passed++;
    } else {
        printf("FAIL  %s\n", name);
        failed++;
    }
}

/*
 * an array with one value: min, max, and mean are all that value
 *
 * Why: n == 1 is the smallest n the stats functions allow. It is the base case
 * of the recursive stats_max, so the recursion must stop here without calling
 * itself. stats_min starts from a[0], and stats_mean divides by n = 1.
 */
static void test_one_value(void) {
    float a[] = {23.5f};
    check(stats_max(a, 1) == 23.5f, "one value: max");
    check(stats_min(a, 1) == 23.5f, "one value: min");
    check(stats_mean(a, 1) == 23.5f, "one value: mean");
}

/*
 * stats_max is correct when the largest value is at index 0, and when it is at index n-1
 *
 * Why: the ends of an array are where a recursion is most likely to be off by
 * one. The two arrays hold the same numbers, and only the position of the max
 * changes. A stats_max that never looks at the first element fails the first
 * check, and one whose base case stops one element early fails the second.
 */
static void test_max_at_ends(void) {
    float first[] = {101.0f, 40.0f, 38.0f, 66.0f};
    float last[]  = {40.0f, 38.0f, 66.0f, 101.0f};

    check(stats_max(first, 4) == 101.0f, "max at index 0");
    check(stats_max(last, 4) == 101.0f, "max at index n-1");
}

/*
 * a value exactly equal to the threshold is not above it, so it ends a run
 *
 * Why: the run counts values strictly above the threshold (>). A value equal
 * to it is the boundary between > and >=. If the comparison were >=, the run
 * would continue through it, and the length and start would be wrong. The
 * starter's edge.txt has 30.0 at tick 360 for the same reason.
 */
static void test_threshold_ends_run(void) {
    /* runs: 31-33 (length 3), then 30.0 breaks it, then 34-35 (length 2) */
    float a[] = {31.0f, 32.0f, 33.0f, 30.0f, 34.0f, 35.0f, 20.0f};
    struct stats_longest_run_result run = stats_longest_run(a, 7, 30.0f);

    check(run.length == 3, "value equal to threshold ends a run: length");
    check(run.start == 0, "value equal to threshold ends a run: start");
}

/*
 * the recursive maximum works on 10,000 values
 *
 * Why: this tests size, not an edge. Each call to stats_max keeps a stack
 * frame until the one below it returns, so 10,000 values means 10,000 frames
 * on the stack at once. It checks that the recursion still gives the right
 * answer that deep. (In lecture, 200,000 values crashed by running out of
 * stack.)
 */
static void test_max_10000(void) {
    float a[10000];
    int value = 1;

    /* a[0] = 1, a[1] = 2, ..., a[9999] = 10000: the max is the last value, so
       it is only found if the recursion really reaches all 10,000 values */
    for (int i = 0; i < 10000; i++) {
        a[i] = (float) value;
        value++;
    }
    check(stats_max(a, 10000) == 10000.0f, "recursive max on 10,000 values");
}

/* MAIN */

int main(void) {
    test_one_value();
    test_max_at_ends();
    test_threshold_ends_run();
    test_max_10000();

    printf("%d passed, %d failed\n", passed, failed);
    return failed == 0 ? 0 : 1;
}
