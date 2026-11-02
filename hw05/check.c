/*
 * check.c: the table checker, ECE 361 HW5 bonus
 *
 * fsm_check runs four helper checks on a transition table and adds up how
 * many problems they found. Each helper prints one line per problem to out.
 *
 *   check_undefined    an entry left out (action is A_UNDEFINED)
 *   check_range        a next state or action outside its enum
 *   check_unreachable  a state that breadth-first search from start never visits
 *   check_dead_end     a state where every event leads back to itself
 *
 * Variables used throughout:
 *   table   the table being checked, passed in from fsm_check; read-only.
 *           table[s][e] is the entry for state s and event e.
 *   out     the open stream every problem line is printed to.
 *   s       loop index over the states, 0 to N_STATES - 1 (row of table).
 *   e       loop index over the events, 0 to N_EVENTS - 1 (column of table).
 *   count   how many problems this function found; returned at the end.
 *
 * Variables used only in check_unreachable (the breadth-first search):
 *   visited[N_STATES]  1 once a state has been reached from start, else 0.
 *   queue[N_STATES]    states reached but not yet explored, oldest first.
 *   head               index of the next state to take out of queue.
 *   tail               index where the next reached state is put into queue.
 *   n                  the next state of the entry being looked at.
 *
 * The helpers read table directly, never lock_step, because lock_step always
 * uses the real lock_table, and fsm_check must also check broken copies.
 */

#include "check.h"      /* fsm_check's prototype, and through it stdio.h and lock.h */

/* Finds entries left out: a left-out entry is zero, so its action is
 * A_UNDEFINED. Prints one line for each and returns how many. */
static int check_undefined(const transition_t table[N_STATES][N_EVENTS], FILE *out) {
    int count = 0;                                  /* problems found so far */

    for (int s = 0; s < N_STATES; s++) {            /* every state (row) */
        for (int e = 0; e < N_EVENTS; e++) {        /* every event (column) */
            if (table[s][e].action == A_UNDEFINED) {    /* left out? */
                fprintf(out, "%s %s: entry left out (A_UNDEFINED)\n",
                        state_name(s), event_name(e));
                count++;
            }
        }
    }
    return count;
}

/* Finds next states and actions outside their enums, such as state 7.
 * Prints one line for each and returns how many. */
static int check_range(const transition_t table[N_STATES][N_EVENTS], FILE *out) {
    int count = 0;                                  /* problems found so far */

    for (int s = 0; s < N_STATES; s++) {            /* every state (row) */
        for (int e = 0; e < N_EVENTS; e++) {        /* every event (column) */
            state_t next = table[s][e].next;        /* where this entry goes */
            action_t action = table[s][e].action;   /* what this entry does */

            if (next < 0 || next >= N_STATES) {     /* not a real state */
                fprintf(out, "%s %s: next state %d out of range\n",
                        state_name(s), event_name(e), (int)next);
                count++;
            }
            if (action < 0 || action >= N_ACTIONS) {    /* not a real action */
                fprintf(out, "%s %s: action %d out of range\n",
                        state_name(s), event_name(e), (int)action);
                count++;
            }
        }
    }
    return count;
}

/* Breadth-first search from start: visits every state the table can get to,
 * then reports each state it never visited. Entries left out or out of range
 * are not followed. Prints one line per unreachable state; returns how many. */
static int check_unreachable(const transition_t table[N_STATES][N_EVENTS],
                             state_t start, FILE *out) {
    int visited[N_STATES] = { 0 };  /* nothing reached yet */
    state_t queue[N_STATES];        /* each state goes in at most once, so N_STATES is enough */
    int head = 0;                   /* next state to explore */
    int tail = 0;                   /* next free slot in queue */
    int count = 0;                  /* problems found so far */

    visited[start] = 1;             /* start is reached by definition */
    queue[tail++] = start;          /* and is the first state to explore */

    while (head < tail) {                           /* until queue is empty */
        state_t s = queue[head++];                  /* take the oldest state */

        for (int e = 0; e < N_EVENTS; e++) {        /* follow each of its events */
            state_t n = table[s][e].next;           /* where that event goes */

            if (table[s][e].action == A_UNDEFINED || n < 0 || n >= N_STATES) {
                continue;                           /* not a real edge: skip */
            }
            if (!visited[n]) {                      /* first time reaching n */
                visited[n] = 1;
                queue[tail++] = n;                  /* explore n later */
            }
        }
    }

    for (int s = 0; s < N_STATES; s++) {            /* every state */
        if (!visited[s]) {                          /* search never got here */
            fprintf(out, "%s: cannot be reached from %s\n",
                    state_name(s), state_name(start));
            count++;
        }
    }
    return count;
}

/* Finds states with no way out: every event's next state is the state
 * itself. Prints one line for each and returns how many. */
static int check_dead_end(const transition_t table[N_STATES][N_EVENTS], FILE *out) {
    int count = 0;                                  /* problems found so far */

    for (int s = 0; s < N_STATES; s++) {            /* every state (row) */
        int stuck = 1;                              /* stuck until a way out is found */

        for (int e = 0; e < N_EVENTS; e++) {        /* every event (column) */
            if (table[s][e].next != (state_t)s) {   /* this event leaves s */
                stuck = 0;                          /* found a way out */
                break;                              /* no need to look further */
            }
        }
        if (stuck) {                                /* all events stayed in s */
            fprintf(out, "%s: no way out (every event stays in %s)\n",
                    state_name(s), state_name(s));
            count++;
        }
    }
    return count;
}

/* Runs all four checks on table; prints one line per problem to out and
 * returns how many problems it found in total. */
int fsm_check(const transition_t table[N_STATES][N_EVENTS], state_t start, FILE *out) {
    int count = 0;                                  /* total over all checks */

    count += check_undefined(table, out);
    count += check_range(table, out);
    count += check_unreachable(table, start, out);  /* the only one that needs start */
    count += check_dead_end(table, out);
    return count;
}
