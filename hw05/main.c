/*
 * main.c: runs the combination lock on standard input
 *
 * Starts in LOCKED, reads one character at a time, and prints one line per
 * event, e.g. "GOT2 4 RIGHT -> OPEN UNLOCK". Characters that are not events
 * (lock_event returns -1), such as newlines, print nothing. At the end it
 * prints "final: STATE".
 *
 * Usage: ./lock < data/codes.txt
 */

#include <stdio.h>       /* getchar, printf, EOF */
#include "lock.h"        /* the lock's types and functions */

int main(void) {                            /* program starts here */
    state_t state = LOCKED;                 /* the lock starts locked */
    int c;                                  /* int, not char, so it can hold EOF */

    while ((c = getchar()) != EOF) {        /* read one char until end of input */
        int e = lock_event(state, (char)c); /* char -> event for this state, or -1 */
        if (e == -1) {                      /* not an event (newline, space, ...) */
            continue;                       /* skip it: back to the next char */
        }                                   /* end of the skip check */

        action_t action;                    /* lock_step writes the action here */
        state_t next = lock_step(state, e, &action); /* table lookup: next state */

        printf("%s %c %s -> %s %s\n",       /* one line per event: */
               state_name(state), c, event_name(e),     /* old state, char, event */
               state_name(next), action_name(action));  /* next state, action */

        state = next;                       /* the lock moves only here */
    }                                       /* end of input loop */

    printf("final: %s\n", state_name(state)); /* state after the last char */
    return 0;                               /* exit status 0: success */
}                                           /* end of main */
