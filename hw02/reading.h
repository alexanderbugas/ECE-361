#ifndef READING_H
#define READING_H

#define MAX_READINGS 1000   /* the most readings the program stores */

/* what reading_read gives back besides the filled arrays */
struct reading_result {
    int count;      /* number of good readings stored */
    int skipped;    /* number of lines that did not hold three numbers */
};

/*
 * reading_read: read readings from standard input, one per line,
 * "tick temperature humidity". Blank lines and lines starting with '#' are
 * ignored and not counted. A line that does not hold three numbers is counted
 * as skipped.
 *
 * Takes:   ticks, temps, hums: arrays to fill; reading i goes in index i of
 *               all three
 *          max: the most readings to store; after that, prints a warning to
 *               stderr and stops reading
 * Returns: the number of readings stored and the number of lines skipped
 * Assumes: each array has room for at least max values
 */
struct reading_result reading_read(int ticks[], float temps[], float hums[], int max);

#endif /* READING_H */
