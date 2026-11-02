#include "lock.h"
static const char code[N_STATES] = { [LOCKED] = '3', [GOT1] = '1', [GOT2] = '4', [OPEN] = '\0' };
static const char *state_names[N_STATES] = {
    [LOCKED] = "LOCKED",
    [GOT1] = "GOT1",
    [GOT2] = "GOT2",
    [OPEN] = "OPEN",
};
static const char *event_names[N_EVENTS] = {
    [E_RIGHT] = "RIGHT",
    [E_WRONG] = "WRONG",
    [E_CLOSE] = "CLOSE",
    [E_TIMEOUT] = "TIMEOUT",
};
static const char *action_names[N_ACTIONS] = {
    [A_UNDEFINED] = "UNDEFINED",
    [A_NONE] = "NONE",
    [A_UNLOCK] = "UNLOCK",
    [A_LOCK] = "LOCK",
    [A_BEEP] = "BEEP",
};

/* Transition table: lock_table[state][event] = { next state, action } */
const transition_t lock_table[N_STATES][N_EVENTS] = {
    [LOCKED] = {
        [E_RIGHT]   = { GOT1, A_NONE },
        [E_WRONG]   = { LOCKED, A_BEEP },
        [E_CLOSE]   = { LOCKED, A_NONE },
        [E_TIMEOUT] = { LOCKED, A_NONE },
    },
    [GOT1] = {
        [E_RIGHT]   = { GOT2, A_NONE },
        [E_WRONG]   = { LOCKED, A_BEEP },
        [E_CLOSE]   = { GOT1, A_NONE },
        [E_TIMEOUT] = { LOCKED, A_NONE },
    },
    [GOT2] = {
        [E_RIGHT]   = { OPEN, A_UNLOCK },
        [E_WRONG]   = { LOCKED, A_BEEP },
        [E_CLOSE]   = { GOT2, A_NONE },
        [E_TIMEOUT] = { LOCKED, A_NONE },
    },
    [OPEN] = {
        [E_RIGHT]   = { OPEN, A_NONE },
        [E_WRONG]   = { OPEN, A_NONE },
        [E_CLOSE]   = { LOCKED, A_LOCK },
        [E_TIMEOUT] = { LOCKED, A_LOCK },
    },
};

/* Looks up lock_table[s][e]; returns the next state, writes the action. */
state_t lock_step(state_t s, event_t e, action_t *action)
{
    *action = lock_table[s][e].action;
    return lock_table[s][e].next;
}

/* Turns character c into an event for state s, or -1 if it is not one. */
int lock_event(state_t s, char c)
{
    if (c >= '0' && c <= '9') {         /* is the input a digit? */
        if (code[s] == c) {             /* is the digit the right one for this state? */
            return E_RIGHT;
        } 
        else {
            return E_WRONG;
        }
    }
    else if (c == 'c') {
        return E_CLOSE;
    } 
    else if (c == 't') {
        return E_TIMEOUT;
    }
    return -1;
}

/* Returns the name of state s, or "?" if out of range. */
const char *state_name(state_t s)
{
    if (s >= 0 && s < N_STATES) {
        return state_names[s];
    }
    return "?";
}

/* Returns the name of event e, or "?" if out of range. */
const char *event_name(event_t e)
{
    if (e >= 0 && e < N_EVENTS) {
        return event_names[e];
    }
    return "?";
}

/* Returns the name of action a, or "?" if out of range. */
const char *action_name(action_t a)
{
    if (a >= 0 && a < N_ACTIONS) {
        return action_names[a];
    }
    return "?";
}
