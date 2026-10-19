/*
 * ptr.c: a small pointer library, ECE 361 HW3
 *
 * Every array is walked with a pointer (p++, p < a + n, *p), never with a[i].
 * The contracts are in ptr.h.
 */
#include <stddef.h>
#include "ptr.h"

/* exchange the two floats a and b point to: t holds the old *a while *a is
   overwritten */
void swap_float(float *a, float *b) {
    if (a == NULL || b == NULL)
        return;

    float t = *a;
    *a = *b;
    *b = t;
}

/* smallest and largest of a[0..n-1]: start both at the first value, walk p
   over the rest, and store the results only once the walk is done */
int minmax(const float *a, int n, float *min, float *max) {
    if (a == NULL || min == NULL || max == NULL || n < 1)
        return -1;

    float lo = *a;
    float hi = *a;

    for (const float *p = a + 1; p < a + n; p++) {
        if (*p < lo)
            lo = *p;
        if (*p > hi)
            hi = *p;
    }

    *min = lo;
    *max = hi;
    return 0;
}

/* first value strictly above threshold: walk p until *p > threshold and hand
   back p itself; falling off the end means there is none */
const float *find_above(const float *a, int n, float threshold) {
    if (a == NULL || n < 1)
        return NULL;

    for (const float *p = a; p < a + n; p++) {
        if (*p > threshold)
            return p;
    }
    return NULL;
}

/* reverse in place: lo starts at the first element, hi at the last; swap
   them and step both toward the middle until they meet or cross */
void reverse(float *a, int n) {
    if (a == NULL || n < 2)
        return;

    float *lo = a;
    float *hi = a + n - 1;

    while (lo < hi) {
        swap_float(lo, hi);
        lo++;
        hi--;
    }
}
