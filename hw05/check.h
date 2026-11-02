/*
 * check.h: the table checker, ECE 361 HW5 bonus
 *
 * One function, fsm_check, that looks at a whole transition table and reports
 * problems with its structure. It does not run the lock.
 *
 * Includes:
 *   <stdio.h>  for FILE, the type of out
 *   "lock.h"   for transition_t, state_t, N_STATES, N_EVENTS
 */

#ifndef CHECK_H
#define CHECK_H

#include <stdio.h>
#include "lock.h"

/*
 * fsm_check: checks table for four kinds of problem and returns how many
 * it found (0 means the table is clean).
 *
 *   table  any table shaped like lock_table: the real one, or a broken copy
 *   start  the state the machine begins in; reachability is measured from it
 *   out    where to print one line per problem (stdout, or a file in tests)
 */
int fsm_check(const transition_t table[N_STATES][N_EVENTS], state_t start, FILE *out);

#endif // CHECK_H
