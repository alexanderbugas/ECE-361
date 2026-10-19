#ifndef PTR_H
#define PTR_H

/*
 * swap_float: exchanges two floats in the caller's memory.
 *
 * Takes:   a, b: the addresses of the two floats to exchange
 * Returns: nothing. Afterwards *a holds the old *b and *b holds the old *a.
 * Assumes: nothing. If a or b is NULL, it does nothing. If a == b, the value
 *          is unchanged.
 */
void swap_float(float *a, float *b);

/*
 * minmax: the smallest and the largest value in an array, in one pass.
 *
 * Takes:   a: the values; they are not changed
 *          n: how many values to look at, starting at a[0]
 *          min, max: where to store the results
 * Returns: 0, with the smallest of a[0] .. a[n-1] in *min and the largest in
 *          *max.
 *          -1 if n < 1 or a, min, or max is NULL. *min and *max are then not
 *          changed.
 * Assumes: a points to at least n floats.
 */
int minmax(const float *a, int n, float *min, float *max);

/*
 * find_above: the first value strictly greater than a threshold. A value
 * equal to the threshold is not above it.
 *
 * Takes:   a: the values; they are not changed
 *          n: how many values to look at, starting at a[0]
 *          threshold: a value must be greater than this to match
 * Returns: a pointer to the first matching value inside the array, or NULL if
 *          no value matches, a is NULL, or n < 1. The returned pointer is
 *          const: the array may not be changed through it.
 * Assumes: a points to at least n floats.
 */
const float *find_above(const float *a, int n, float threshold);

/*
 * reverse: reverses an array in place.
 *
 * Takes:   a: the values; they are changed
 *          n: how many values to reverse, starting at a[0]
 * Returns: nothing. Afterwards a[0] holds the old a[n-1], a[1] the old
 *          a[n-2], and so on.
 * Assumes: a points to at least n floats. If a is NULL or n < 2, it does
 *          nothing.
 */
void reverse(float *a, int n);

#endif /* PTR_H */
