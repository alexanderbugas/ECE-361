#include "histogram.h"
#include <stdio.h>

#define NUM_BINS     20          /* histogram: 0 to 100 C in 5 C bins */
#define BIN_WIDTH    5.0f

/* count the values into bins, then print each non-empty bin; see histogram.h */
void histogram_print(const float a[], int n) {
    int bins[NUM_BINS] = {0};
    for (int i = 0; i < n; i++) {
        int b = (int) (a[i] / BIN_WIDTH);
        if (a[i] < 0.0f)
            b = 0;
        if (b >= NUM_BINS)
            b = NUM_BINS - 1;
        bins[b]++;
    }
    printf("histogram:\n");
    for (int b = 0; b < NUM_BINS; b++) {
        if (bins[b] == 0)
            continue;
        printf("  %5.1f to %5.1f C | ", b * BIN_WIDTH, (b + 1) * BIN_WIDTH);
        for (int k = 0; k < bins[b]; k++)
            putchar('*');
        printf(" %d\n", bins[b]);
    }
}