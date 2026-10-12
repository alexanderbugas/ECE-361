#ifndef HISTOGRAM_H
#define HISTOGRAM_H

/*
 * histogram_print: print a histogram of temperatures to standard output, in
 * 5 C bins from 0 to 100 C. Values below 0 go in the first bin, values of 100
 * or more in the last. Only bins that hold at least one value are printed,
 * one row of '*' per bin followed by the count.
 *
 * Takes:   a: the temperatures; it is not changed
 *          n: how many values to count, starting at index 0
 * Returns: nothing
 * Assumes: n >= 0. With n == 0 it prints only the "histogram:" line.
 */
void histogram_print(const float a[], int n);

#endif /* HISTOGRAM_H */
