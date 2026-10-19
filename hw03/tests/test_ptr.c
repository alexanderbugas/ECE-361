/*
 * test_ptr.c: unit tests for the pointer library, ECE 361 HW3
 *
 * Prints PASS or FAIL for each test and a summary.
 * Exits with 1 if any test fails, so that make test fails.
 */
#include <stdio.h>
#include "ptr.h"

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
 * swap_float exchanges the caller's two floats, and does nothing with NULL
 *
 * Why: this is the point of passing addresses. If swap_float took plain
 * floats, it would swap its own copies and x and y here would not change.
 * The NULL half checks the user's decision: a NULL pointer means "do
 * nothing", so x must keep its value instead of the program crashing.
 */
static void test_swap(void) {
    float x = 1.5f;
    float y = -2.0f;

    swap_float(&x, &y);
    check(x == -2.0f && y == 1.5f, "swap_float exchanges two values");

    swap_float(&x, NULL);
    check(x == -2.0f, "swap_float with NULL changes nothing");
}

/*
 * minmax on several values, all negative, with the min in the middle and the
 * max at the last index
 *
 * Why: all negative values catch the bug from the first lecture's max.c: a
 * minmax that starts from 0 instead of a[0] reports a max of 0, a number not
 * in the array. Putting the max at a + n - 1 catches a loop that stops one
 * element early.
 */
static void test_minmax_several(void) {
    float a[] = {-5.0f, -9.0f, -3.5f, -2.0f};
    float lo = 999.0f;
    float hi = 999.0f;

    check(minmax(a, 4, &lo, &hi) == 0, "minmax several values: returns 0");
    check(lo == -9.0f, "minmax several values: min");
    check(hi == -2.0f, "minmax several values: max");
}

/*
 * minmax on one value: min and max are both that value
 *
 * Why: n == 1 is the smallest n allowed. The loop starts at a + 1, which
 * equals a + n, so the loop body never runs and the answer comes from the
 * starting value alone.
 */
static void test_minmax_one(void) {
    float a[] = {23.5f};
    float lo = 999.0f;
    float hi = 999.0f;

    check(minmax(a, 1, &lo, &hi) == 0, "minmax one value: returns 0");
    check(lo == 23.5f && hi == 23.5f, "minmax one value: min and max");
}

/*
 * minmax with n == 0 returns -1 and leaves the outputs alone
 *
 * Why: with no values there is no correct answer. lo and hi start at 999, a
 * value no real result here could be, so if minmax stored anything at all
 * the check sees it.
 */
static void test_minmax_empty(void) {
    float a[] = {1.0f};
    float lo = 999.0f;
    float hi = 999.0f;

    check(minmax(a, 0, &lo, &hi) == -1, "minmax n = 0: returns -1");
    check(lo == 999.0f && hi == 999.0f, "minmax n = 0: outputs unchanged");
}

/*
 * minmax with a NULL pointer returns -1 and leaves the outputs alone
 *
 * Why: first a NULL array: following it would crash. Then a NULL min with a
 * valid max: minmax must refuse the whole call, not fill in the half it can,
 * so hi must also stay 999.
 */
static void test_minmax_null(void) {
    float a[] = {1.0f, 2.0f};
    float lo = 999.0f;
    float hi = 999.0f;

    check(minmax(NULL, 2, &lo, &hi) == -1, "minmax NULL array: returns -1");
    check(lo == 999.0f && hi == 999.0f, "minmax NULL array: outputs unchanged");

    check(minmax(a, 2, NULL, &hi) == -1, "minmax NULL min: returns -1");
    check(hi == 999.0f, "minmax NULL min: max unchanged");
}

/*
 * find_above with two matches returns the first one
 *
 * Why: the two matches hold the same value, 31.0, so checking only *result
 * cannot tell them apart. The test compares the returned pointer with
 * &a[1], the address of the first match, which catches a version that keeps
 * walking and returns the last match instead.
 */
static void test_find_two_matches(void) {
    float a[] = {20.0f, 31.0f, 25.0f, 31.0f};
    const float *result = find_above(a, 4, 30.0f);

    check(result == &a[1], "find_above two matches: returns the first");
}

/*
 * find_above with no value above the threshold returns NULL
 *
 * Why: NULL is how find_above says "not found". The largest value, 29.9, is
 * just below the threshold, and it is the last element, so the loop has to
 * read all n values and stop at a + n without reading past it.
 */
static void test_find_none(void) {
    float a[] = {20.0f, 25.0f, 29.9f};

    check(find_above(a, 3, 30.0f) == NULL, "find_above no match: returns NULL");
}

/*
 * a value equal to the threshold is not above it
 *
 * Why: the comparison is strictly > (the same boundary as the longest run in
 * HW2). 30.0 at index 1 equals the threshold and must be skipped, so the
 * answer is 30.5 at index 2. With >= instead of >, find_above would return
 * &a[1].
 */
static void test_find_equal(void) {
    float a[] = {10.0f, 30.0f, 30.5f};

    check(find_above(a, 3, 30.0f) == &a[2], "find_above value equal to threshold: skipped");
}

/*
 * find_above with a NULL array or n == 0 returns NULL
 *
 * Why: this tests the user's decision: bad input gives the same answer as
 * "not found" instead of crashing. With n == 0 the array exists, but there
 * is nothing to look at, even though a[0] would match.
 */
static void test_find_null(void) {
    float a[] = {50.0f};

    check(find_above(NULL, 3, 30.0f) == NULL, "find_above NULL array: returns NULL");
    check(find_above(a, 0, 30.0f) == NULL, "find_above n = 0: returns NULL");
}

/*
 * reverse on an even length: every element ends up in its mirror position
 *
 * Why: with 4 values, lo and hi swap twice and then cross (lo passes hi), and
 * the loop must stop there. A loop that keeps going until lo reaches the end
 * swaps every pair a second time and puts the array back the way it was.
 * Every element is checked, not only the ends.
 */
static void test_reverse_even(void) {
    float a[] = {1.0f, 2.0f, 3.0f, 4.0f};

    reverse(a, 4);
    check(a[0] == 4.0f && a[1] == 3.0f && a[2] == 2.0f && a[3] == 1.0f,
          "reverse even length (4)");
}

/*
 * reverse on an odd length: the middle element stays where it is
 *
 * Why: with 5 values, lo and hi swap twice and then meet on the middle
 * element, a[2]. lo < hi is false when they are equal, so the middle is
 * never swapped with itself, and it must still hold 3.0.
 */
static void test_reverse_odd(void) {
    float a[] = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f};

    reverse(a, 5);
    check(a[0] == 5.0f && a[1] == 4.0f && a[2] == 3.0f && a[3] == 2.0f &&
          a[4] == 1.0f, "reverse odd length (5)");
}

/*
 * reverse does nothing with n < 2 or a NULL array
 *
 * Why: this checks the contract: n == 1 has nothing to reverse, n == 0 has
 * nothing at all, and a NULL array must not crash. The array must be left as
 * it was.
 * What it cannot check: whether reverse has its own guard. Without it, n == 0
 * would set hi = a - 1 (undefined behavior, even unread), but the loop would
 * not run, and with NULL, swap_float's own guard would absorb the calls. The
 * result looks the same either way, so the guard is there for correctness,
 * not for this test.
 */
static void test_reverse_nothing(void) {
    float a[] = {1.0f, 2.0f};

    reverse(a, 1);
    reverse(a, 0);
    reverse(NULL, 2);
    check(a[0] == 1.0f && a[1] == 2.0f, "reverse n < 2 and NULL: nothing changes");
}

/* MAIN */

int main(void) {
    test_swap();
    test_minmax_several();
    test_minmax_one();
    test_minmax_empty();
    test_minmax_null();
    test_find_two_matches();
    test_find_none();
    test_find_equal();
    test_find_null();
    test_reverse_even();
    test_reverse_odd();
    test_reverse_nothing();

    printf("%d passed, %d failed\n", passed, failed);
    return failed == 0 ? 0 : 1;
}
