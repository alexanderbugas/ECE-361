/*
 * test_all.c: runs every module's unit tests and prints one summary line
 *
 * Each tests/test_<module>.c holds the tests for one module and exposes one
 * function, declared here. Exits with 1 if any test fails, so make test fails.
 */
#include "check.h"

void test_fsm(void);    /* tests/test_fsm.c */
void test_bits(void);   /* tests/test_bits.c */
void test_store(void);  /* tests/test_store.c */
void test_sensor(void); /* tests/test_sensor.c */
void test_scenario(void); /* tests/test_scenario.c */

int main(void) {
    test_fsm();
    test_bits();
    test_store();
    test_sensor();
    test_scenario();
    return check_report();
}
