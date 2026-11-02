/*
 * test_check.c: unit tests for the table checker, ECE 361 HW5 bonus
 *
 * Prints PASS or FAIL for each test and a summary.
 * Exits with 1 if any test fails, so that make bonus fails.
 *
 * How it uses the checker:
 *   This file includes only check.h, which brings in lock.h. From check.h it
 *   uses fsm_check; from lock.h it uses lock_table, the types, and the
 *   N_STATES / N_EVENTS sizes. It is linked with check.o (fsm_check) and
 *   lock.o (lock_table and the name functions fsm_check prints with); see
 *   the bonus rules in the Makefile.
 *
 * What it passes to fsm_check(table, start, out):
 *   table  lock_table itself, or t, a local copy of it broken on purpose
 *   start  always LOCKED, where the lock begins
 *   out    stdout, so the checker's lines appear under each test's heading
 *          and can be copied into BONUS.md
 *   fsm_check returns how many problems it found; each test compares that
 *   number with the count it expects for the break it made.
 *
 * Variables:
 *   passed, failed  how many checks passed and failed; printed in the summary
 *                   and used for the exit status.
 *   t               in each test, a writable copy of lock_table
 *                   (transition_t t[N_STATES][N_EVENTS]); lock_table is
 *                   const, so the test breaks the copy instead.
 *   n               in test_real_table, the count fsm_check returned.
 *
 * Each test: copy lock_table into t, change one or two entries, run
 * fsm_check on t, and check the count.
 */

#include <stdio.h>      /* printf, stdout */
#include <string.h>     /* memcpy, memset */
#include "check.h"      /* fsm_check, and through lock.h: lock_table and the types */

static int passed = 0;      /* checks that passed */
static int failed = 0;      /* checks that failed; nonzero makes main return 1 */

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

/* copies lock_table into t so a test can break the copy */
static void copy_table(transition_t t[N_STATES][N_EVENTS]) {
    memcpy(t, lock_table, sizeof lock_table);  /* all 16 entries, byte for byte */
}

/* runs fsm_check on t from LOCKED, printing its lines under a heading */
static int run_check(const char *heading, const transition_t t[N_STATES][N_EVENTS]) {
    printf("--- %s\n", heading);              /* which table the lines below are about */
    return fsm_check(t, LOCKED, stdout);       /* table t, start LOCKED, print to stdout */
}

/*
 * The real table has no problems
 *
 * Why: a checker that complains about a correct table is no use.
 */
static void test_real_table(void) {
    int n = run_check("real lock_table", lock_table);   /* the unbroken table */

    check(n == 0, "check: real lock_table has 0 problems");
}

/*
 * GOT2 RIGHT left out: the entry is undefined, and OPEN can no longer be
 * reached
 *
 * Why: a left-out entry is zero, { LOCKED, A_UNDEFINED }, so it is found by
 * its action, and BFS must not follow it as if it were a real edge.
 */
static void test_left_out(void) {
    transition_t t[N_STATES][N_EVENTS];    /* writable copy to break */

    copy_table(t);
    t[GOT2][E_RIGHT] = (transition_t){ 0 };    /* as if the line were never written */
    check(run_check("GOT2 RIGHT left out", t) == 2,
          "check: GOT2 RIGHT left out gives 2 problems");
}

/*
 * GOT1 RIGHT goes to state 7: out of range, and GOT2 and OPEN can no longer
 * be reached
 *
 * Why: BFS must skip a next state out of range instead of indexing past
 * the end of visited.
 */
static void test_out_of_range(void) {
    transition_t t[N_STATES][N_EVENTS];    /* writable copy to break */

    copy_table(t);
    t[GOT1][E_RIGHT].next = (state_t)7;        /* a state that does not exist */
    check(run_check("GOT1 RIGHT goes to state 7", t) == 3,
          "check: GOT1 RIGHT to state 7 gives 3 problems");
}

/*
 * An action out of range is found too
 *
 * Why: action_name would print "?" for it, and nothing downstream knows what
 * to do with it.
 */
static void test_action_out_of_range(void) {
    transition_t t[N_STATES][N_EVENTS];    /* writable copy to break */

    copy_table(t);
    t[LOCKED][E_WRONG].action = (action_t)9;   /* an action that does not exist */
    check(run_check("LOCKED WRONG action 9", t) == 1,
          "check: LOCKED WRONG action 9 gives 1 problem");
}

/*
 * GOT2 RIGHT stays in GOT2: OPEN cannot be reached, and nothing else is wrong
 *
 * Why: this table is complete and in range, so only BFS can find it.
 */
static void test_unreachable(void) {
    transition_t t[N_STATES][N_EVENTS];    /* writable copy to break */

    copy_table(t);
    t[GOT2][E_RIGHT].next = GOT2;              /* the last digit no longer opens */
    check(run_check("GOT2 RIGHT stays in GOT2", t) == 1,
          "check: GOT2 RIGHT stays in GOT2 gives 1 problem");
}

/*
 * OPEN CLOSE and OPEN TIMEOUT stay in OPEN: OPEN has no way out
 *
 * Why: every event in OPEN now leads back to OPEN.
 */
static void test_dead_end(void) {
    transition_t t[N_STATES][N_EVENTS];    /* writable copy to break */

    copy_table(t);
    t[OPEN][E_CLOSE].next = OPEN;              /* closing leaves it open */
    t[OPEN][E_TIMEOUT].next = OPEN;            /* and so does the timer */
    check(run_check("OPEN CLOSE and TIMEOUT stay in OPEN", t) == 1,
          "check: OPEN with no way out gives 1 problem");
}

/*
 * A table with every entry left out
 *
 * Why: all 16 are undefined, GOT1, GOT2 and OPEN cannot be reached, and
 * LOCKED is stuck, because every zero entry points back to LOCKED: 20 lines.
 */
static void test_empty(void) {
    transition_t t[N_STATES][N_EVENTS];    /* table to empty */

    memset(t, 0, sizeof t);                    /* every entry { LOCKED, A_UNDEFINED } */
    check(run_check("every entry left out", t) == 20,
          "check: empty table gives 20 problems");
}

/* runs every test, prints the summary, and returns 1 if any check failed */
int main(void) {
    test_real_table();
    test_left_out();
    test_out_of_range();
    test_action_out_of_range();
    test_unreachable();
    test_dead_end();
    test_empty();

    printf("\n%d passed, %d failed\n", passed, failed);
    return failed > 0 ? 1 : 0;     /* nonzero exit makes make bonus fail */
}
