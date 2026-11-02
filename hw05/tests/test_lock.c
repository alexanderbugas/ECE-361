/*
 * test_lock.c: unit tests for the combination lock
 *
 * Prints PASS or FAIL for each test and a summary.
 * Exits with 1 if any test fails, so that make test fails.
 *
 * Like main.c, this file includes only lock.h and uses the lock only through
 * its functions.
 */

/*
Tests, at least:
-every one of the 16 table entries;
-lock_event for a right and a wrong digit in each state;
-lock_event while open;
-lock_event for c, t, and other characters;
-the sequences 3 1 4 (opens), 3 3 1 4 (does not), 3 1 t 4 (does not),
 and 3 1 4 c (locks again);

-PASS or FAIL per test;
-a summary;
-a nonzero exit status if any test fails;
*/

#include <stdio.h>
#include "lock.h"

static int passed = 0;
static int failed = 0;

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

/*
 * Every one of the 16 entries: lock_step returns the expected next state and
 * writes the expected action
 *
 * Why: the expected values are written out here, separately from lock_table,
 * so a typo in lock.c shows up as a mismatch instead of being copied.
 */
static void test_table_entries(void) {
    static const transition_t expected[N_STATES][N_EVENTS] = {
        [LOCKED] = {
            [E_RIGHT]   = { GOT1,   A_NONE   },
            [E_WRONG]   = { LOCKED, A_BEEP   },
            [E_CLOSE]   = { LOCKED, A_NONE   },
            [E_TIMEOUT] = { LOCKED, A_NONE   },
        },
        [GOT1] = {
            [E_RIGHT]   = { GOT2,   A_NONE   },
            [E_WRONG]   = { LOCKED, A_BEEP   },
            [E_CLOSE]   = { GOT1,   A_NONE   },
            [E_TIMEOUT] = { LOCKED, A_NONE   },
        },
        [GOT2] = {
            [E_RIGHT]   = { OPEN,   A_UNLOCK },
            [E_WRONG]   = { LOCKED, A_BEEP   },
            [E_CLOSE]   = { GOT2,   A_NONE   },
            [E_TIMEOUT] = { LOCKED, A_NONE   },
        },
        [OPEN] = {
            [E_RIGHT]   = { OPEN,   A_NONE   },
            [E_WRONG]   = { OPEN,   A_NONE   },
            [E_CLOSE]   = { LOCKED, A_LOCK   },
            [E_TIMEOUT] = { LOCKED, A_LOCK   },
        },
    };

    for (int s = 0; s < N_STATES; s++) {
        for (int e = 0; e < N_EVENTS; e++) {
            action_t action;
            state_t next = lock_step(s, e, &action);
            char name[64];

            snprintf(name, sizeof name, "table: %s %s -> %s %s",
                     state_name(s), event_name(e),
                     state_name(expected[s][e].next),
                     action_name(expected[s][e].action));
            check(next == expected[s][e].next &&
                  action == expected[s][e].action, name);
        }
    }
}

/*
 * A right and a wrong digit in each state that is typing a code
 *
 * Why: the same key is right in one state and wrong in another, so each
 * state is checked against its own digit of 3-1-4.
 */
static void test_event_digits(void) {
    check(lock_event(LOCKED, '3') == E_RIGHT, "event: LOCKED 3 is RIGHT");
    check(lock_event(LOCKED, '1') == E_WRONG, "event: LOCKED 1 is WRONG");
    check(lock_event(GOT1, '1') == E_RIGHT,   "event: GOT1 1 is RIGHT");
    check(lock_event(GOT1, '3') == E_WRONG,   "event: GOT1 3 is WRONG");
    check(lock_event(GOT2, '4') == E_RIGHT,   "event: GOT2 4 is RIGHT");
    check(lock_event(GOT2, '9') == E_WRONG,   "event: GOT2 9 is WRONG");
}

/*
 * While open, every digit is WRONG, even the digits of the code
 *
 * Why: there is no next digit while open, so nothing can be RIGHT.
 */
static void test_event_open(void) {
    int all_wrong = 1;

    for (char c = '0'; c <= '9'; c++) {
        if (lock_event(OPEN, c) != E_WRONG) {
            all_wrong = 0;
        }
    }
    check(all_wrong, "event: OPEN every digit is WRONG");
}

/*
 * c is CLOSE and t is TIMEOUT in every state; anything else is -1
 *
 * Why: c and t do not depend on the state, and characters like the newline
 * at the end of each line in codes.txt must be ignored, not count as WRONG.
 */
static void test_event_other(void) {
    int c_ok = 1;
    int t_ok = 1;

    for (int s = 0; s < N_STATES; s++) {
        if (lock_event(s, 'c') != E_CLOSE) {
            c_ok = 0;
        }
        if (lock_event(s, 't') != E_TIMEOUT) {
            t_ok = 0;
        }
    }
    check(c_ok, "event: c is CLOSE in every state");
    check(t_ok, "event: t is TIMEOUT in every state");
    check(lock_event(LOCKED, '\n') == -1, "event: newline is -1");
    check(lock_event(LOCKED, ' ') == -1,  "event: space is -1");
    check(lock_event(LOCKED, 'x') == -1,  "event: x is -1");
    check(lock_event(LOCKED, 'C') == -1,  "event: C (uppercase) is -1");
}

/*
 * Runs keys through the lock from LOCKED, the way main does, and returns the
 * final state; *last is set to the action of the last event
 */
static state_t run(const char *keys, action_t *last) {
    state_t s = LOCKED;

    *last = A_UNDEFINED;
    for (const char *p = keys; *p != '\0'; p++) {
        int e = lock_event(s, *p);
        if (e == -1) {
            continue;
        }
        s = lock_step(s, e, last);
    }
    return s;
}

/*
 * The four sequences from the assignment
 *
 * Why: these check that the entries work together, not just one at a time.
 */
static void test_sequences(void) {
    action_t last;
    state_t s;

    s = run("314", &last);
    check(s == OPEN && last == A_UNLOCK, "sequence: 3 1 4 opens");

    s = run("3314", &last);
    check(s == LOCKED, "sequence: 3 3 1 4 does not open");

    s = run("31t4", &last);
    check(s == LOCKED, "sequence: 3 1 t 4 does not open");

    s = run("314c", &last);
    check(s == LOCKED && last == A_LOCK, "sequence: 3 1 4 c locks again");
}

int main(void) {
    test_table_entries();
    test_event_digits();
    test_event_open();
    test_event_other();
    test_sequences();

    printf("\n%d passed, %d failed\n", passed, failed);
    return failed > 0 ? 1 : 0;
}
