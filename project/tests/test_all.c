/*
 * test_all.c: runs every module's unit tests and prints one summary line
 *
 * Each tests/test_<module>.c holds the tests for one module and exposes one
 * function, declared here. Exits with 1 if any test fails, so make test fails.
 */
#include "check.h"

void test_fsm(void);    /* tests/test_fsm.c */
void test_bits(void);   /* tests/test_bits.c */

int main(void) {
    test_fsm();
    test_bits();
    return check_report();
}
