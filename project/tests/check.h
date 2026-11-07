/*
 * check.h: the test helper shared by every test file
 *
 * check() records one result and prints PASS or FAIL with its name, as in
 * hw05/tests/test_lock.c. check_report() prints the one summary line that
 * make test ends with, and gives the exit status.
 */
#ifndef CHECK_H
#define CHECK_H

/* records one result: cond is nonzero if the test passed */
void check(int cond, const char *name);

/* prints "N passed, M failed"; returns 1 if any test failed, else 0 */
int check_report(void);

#endif /* CHECK_H */
