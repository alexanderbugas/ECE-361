#include <stdio.h>
#include "check.h"

static int passed = 0;
static int failed = 0;

/* record one result: cond is nonzero if the test passed */
void check(int cond, const char *name) {
    if (cond) {
        printf("PASS  %s\n", name);
        passed++;
    } else {
        printf("FAIL  %s\n", name);
        failed++;
    }
}

/* the summary line; nonzero if anything failed, so make test fails */
int check_report(void) {
    printf("\n%d passed, %d failed\n", passed, failed);
    return failed > 0 ? 1 : 0;
}
