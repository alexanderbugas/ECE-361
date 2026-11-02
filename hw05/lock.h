#ifndef LOCK_H
#define LOCK_H

typedef enum { LOCKED, GOT1, GOT2, OPEN, N_STATES } state_t;                            /* STATES  */
typedef enum { E_RIGHT, E_WRONG, E_CLOSE, E_TIMEOUT, N_EVENTS } event_t;                /* EVENTS  */
typedef enum { A_UNDEFINED, A_NONE, A_UNLOCK, A_LOCK, A_BEEP, N_ACTIONS } action_t;     /* ACTIONS */
typedef struct { state_t next; action_t action; } transition_t;                         /* TRANSITIONS */

extern const transition_t lock_table[N_STATES][N_EVENTS];                               /* TRANSITION TABLE */
state_t lock_step(state_t s, event_t e, action_t *action);                              /* LOCK STEP */
int lock_event(state_t s, char c);                                                      /* LOCK EVENT */
const char *state_name(state_t s);                                                      /* NAME OF STATE s, "?" IF OUT OF RANGE */
const char *event_name(event_t e);                                                      /* NAME OF EVENT e, "?" IF OUT OF RANGE */
const char *action_name(action_t a);                                                    /* NAME OF ACTION a, "?" IF OUT OF RANGE */

#endif // LOCK_H