#include "fsm.h"
#include "bits.h"       /* get_field, set_field */

#define WIND_ALARM_BIT  0x1u    /* bit 0 of SWITCHES_REG */
#define MOTOR_TICKS     20      /* rule 2 */
#define STORM_TICKS     120     /* rule 3 */
#define HOT_T10         280     /* 28.0 C, in tenths (rule 1) */
#define COOL_T10        240     /* 24.0 C */
#define HUMID_H10       850     /* 85 %RH */
#define DRY_H10         750     /* 75 %RH */

static const char *state_names[N_STATES] = {
    [CLOSED]     = "CLOSED",
    [OPENING]    = "OPENING",
    [OPEN]       = "OPEN",
    [CLOSING]    = "CLOSING",
    [STORM_LOCK] = "STORM_LOCK",
};
static const char *event_names[N_EVENTS] = {
    [E_COOL_ENABLE]        = "COOL_ENABLE",
    [E_HUMID_TRIGGER_HI]   = "HUMID_TRIGGER_HI",
    [E_COOL_DRY]           = "COOL_DRY",
    [E_MOTOR_OPEN_20]      = "MOTOR_OPEN_20",
    [E_MOTOR_CLOSE_20]     = "MOTOR_CLOSE_20",
    [E_WIND_ALARM_ON]      = "WIND_ALARM_ON",
    [E_WIND_ALARM_OFF_120] = "WIND_ALARM_OFF_120",
};
static const char *action_names[N_ACTIONS] = {
    [A_UNDEFINED]   = "UNDEFINED",
    [A_NONE]        = "NONE",
    [A_START_OPEN]  = "START_OPEN",
    [A_START_CLOSE] = "START_CLOSE",
    [A_STOP]        = "STOP",
    [A_STORM]       = "STORM",
    [A_STORM_CLEAR] = "STORM_CLEAR",
};

/* Transition table (SPEC.md): fsm_table[state][event] = { next, action }.
   Cells that cannot happen (for example the motor finishing while CLOSED)
   stay in the same state with A_NONE, so no cell is left out. */
const transition_t fsm_table[N_STATES][N_EVENTS] = {
    [CLOSED] = {
        [E_COOL_ENABLE]        = { OPENING, A_START_OPEN },
        [E_HUMID_TRIGGER_HI]   = { OPENING, A_START_OPEN },
        [E_COOL_DRY]           = { CLOSED, A_NONE },
        [E_MOTOR_OPEN_20]      = { CLOSED, A_NONE },
        [E_MOTOR_CLOSE_20]     = { CLOSED, A_NONE },
        [E_WIND_ALARM_ON]      = { STORM_LOCK, A_STORM },
        [E_WIND_ALARM_OFF_120] = { CLOSED, A_NONE },
    },
    [OPENING] = {
        [E_COOL_ENABLE]        = { OPENING, A_NONE },
        [E_HUMID_TRIGGER_HI]   = { OPENING, A_NONE },
        [E_COOL_DRY]           = { OPENING, A_NONE },       /* silent rule 1: finish opening */
        [E_MOTOR_OPEN_20]      = { OPEN, A_STOP },
        [E_MOTOR_CLOSE_20]     = { OPENING, A_NONE },
        [E_WIND_ALARM_ON]      = { CLOSING, A_START_CLOSE }, /* silent rule 4 */
        [E_WIND_ALARM_OFF_120] = { OPENING, A_NONE },
    },
    [OPEN] = {
        [E_COOL_ENABLE]        = { OPEN, A_NONE },
        [E_HUMID_TRIGGER_HI]   = { OPEN, A_NONE },
        [E_COOL_DRY]           = { CLOSING, A_START_CLOSE },
        [E_MOTOR_OPEN_20]      = { OPEN, A_NONE },
        [E_MOTOR_CLOSE_20]     = { OPEN, A_NONE },
        [E_WIND_ALARM_ON]      = { CLOSING, A_START_CLOSE }, /* silent rule 4 */
        [E_WIND_ALARM_OFF_120] = { OPEN, A_NONE },
    },
    [CLOSING] = {
        [E_COOL_ENABLE]        = { CLOSING, A_NONE },       /* silent rule 2: finish closing */
        [E_HUMID_TRIGGER_HI]   = { CLOSING, A_NONE },
        [E_COOL_DRY]           = { CLOSING, A_NONE },
        [E_MOTOR_OPEN_20]      = { CLOSING, A_NONE },
        [E_MOTOR_CLOSE_20]     = { CLOSED, A_STOP },
        [E_WIND_ALARM_ON]      = { CLOSING, A_NONE },       /* silent rule 5: keep closing */
        [E_WIND_ALARM_OFF_120] = { CLOSING, A_NONE },
    },
    [STORM_LOCK] = {
        [E_COOL_ENABLE]        = { STORM_LOCK, A_NONE },
        [E_HUMID_TRIGGER_HI]   = { STORM_LOCK, A_NONE },
        [E_COOL_DRY]           = { STORM_LOCK, A_NONE },
        [E_MOTOR_OPEN_20]      = { STORM_LOCK, A_NONE },
        [E_MOTOR_CLOSE_20]     = { STORM_LOCK, A_NONE },
        [E_WIND_ALARM_ON]      = { STORM_LOCK, A_NONE },
        [E_WIND_ALARM_OFF_120] = { CLOSED, A_STORM_CLEAR },  /* silent rule 7 */
    },
};

/* name of state s, or "?" if out of range */
const char *state_name(state_t s) {
    if (s >= 0 && s < N_STATES) {
        return state_names[s];
    }
    return "?";
}

/* name of event e, or "?" if out of range */
const char *event_name(event_t e) {
    if (e >= 0 && e < N_EVENTS) {
        return event_names[e];
    }
    return "?";
}

/* name of action a, or "?" if out of range */
const char *action_name(action_t a) {
    if (a >= 0 && a < N_ACTIONS) {
        return action_names[a];
    }
    return "?";
}

/* looks up fsm_table[s][e]; returns the next state, writes the action */
state_t fsm_step(state_t s, event_t e, action_t *action) {
    if (s < 0 || s >= N_STATES || e < 0 || e >= N_EVENTS) {
        *action = A_UNDEFINED;      /* not a real cell: stay where we are */
        return s;
    }
    *action = fsm_table[s][e].action;
    return fsm_table[s][e].next;
}

/* a float that holds a whole number of tenths, back to those tenths:
   round half away from zero, which is right below 0 C too */
static int to_tenths(float x) {
    return (int)(x * 10.0f + (x >= 0.0f ? 0.5f : -0.5f));
}

/* nothing has happened yet at the start of a run */
void fsm_timers_init(fsm_timers_t *tm, time_t now) {
    tm->motor_start = now;
    tm->alarm_off_since = now;
    tm->alarm_was_on = 0;
    tm->storm_pending = 0;
}

/* the motor events: the 20 ticks are up while the state still waits */
static int motor_event(const fsm_timers_t *tm, state_t s, time_t now) {
    if (now - tm->motor_start < MOTOR_TICKS) {
        return -1;
    }
    if (s == OPENING) {
        return E_MOTOR_OPEN_20;
    }
    if (s == CLOSING) {
        return E_MOTOR_CLOSE_20;
    }
    return -1;
}

/* the wind events: the alarm, a storm not locked yet, or 120 ticks off */
static int wind_event(const fsm_timers_t *tm, state_t s, time_t now, int alarm) {
    if (alarm || tm->storm_pending) {
        return E_WIND_ALARM_ON;
    }
    if (s == STORM_LOCK && now - tm->alarm_off_since >= STORM_TICKS) {
        return E_WIND_ALARM_OFF_120;
    }
    return -1;
}

/* the climate events, in whole tenths; at most one of the three groups holds */
static int climate_event(const reading_t *r) {
    int t10 = to_tenths(r->temp_c);
    int h10 = to_tenths(r->rh_pct);

    if (t10 >= HOT_T10) {
        return E_COOL_ENABLE;
    }
    if (h10 >= HUMID_H10) {
        return E_HUMID_TRIGGER_HI;
    }
    if (t10 <= COOL_T10 && h10 <= DRY_H10) {
        return E_COOL_DRY;
    }
    return -1;
}

/* priority: motor, then wind, then climate (SPEC.md, "Events") */
int fsm_event(const fsm_timers_t *tm, state_t s, const reading_t *r, uint32_t switches) {
    int alarm = (switches & WIND_ALARM_BIT) != 0;
    int e = motor_event(tm, s, r->t);

    if (e == -1) {
        e = wind_event(tm, s, r->t, alarm);
    }
    if (e == -1) {
        e = climate_event(r);
    }
    return e;
}

/* the alarm memory: when it turned off, and a storm waiting to be locked */
static void update_alarm(fsm_timers_t *tm, uint32_t switches, time_t now) {
    int alarm = (switches & WIND_ALARM_BIT) != 0;

    if (alarm) {
        tm->storm_pending = 1;
    } else if (tm->alarm_was_on) {
        tm->alarm_off_since = now;      /* the tick it turned off */
    }
    tm->alarm_was_on = alarm;
}

/* one tick: alarm memory, event, step, then the timers that follow the step */
state_t fsm_tick(fsm_timers_t *tm, state_t s, const reading_t *r, uint32_t switches,
                 int *event, action_t *action) {
    update_alarm(tm, switches, r->t);

    int e = fsm_event(tm, s, r, switches);
    if (event != NULL) {
        *event = e;
    }
    if (e == -1) {                      /* no event: no step, as in HW5 */
        *action = A_NONE;
        return s;
    }

    state_t next = fsm_step(s, (event_t)e, action);
    if (next != s && (next == OPENING || next == CLOSING)) {
        tm->motor_start = r->t;         /* the motor starts on this tick */
    }
    if (next == STORM_LOCK) {
        tm->storm_pending = 0;          /* the storm is locked */
    }
    return next;
}

/* one set_field per field, starting from 0 so unused bits stay 0 */
uint32_t status_pack(state_t s, int motor_open, int motor_close, int wind) {
    uint32_t word = 0;

    /* != 0 turns any "on" value into 1; set_field would cut 2 to 0 */
    word = set_field(word, STATUS_MOTOR_OPEN_POS, STATUS_FLAG_WIDTH, motor_open != 0);
    word = set_field(word, STATUS_MOTOR_CLOSE_POS, STATUS_FLAG_WIDTH, motor_close != 0);
    word = set_field(word, STATUS_WIND_POS, STATUS_FLAG_WIDTH, wind != 0);
    word = set_field(word, STATUS_STATE_POS, STATUS_STATE_WIDTH, (uint32_t)s);
    return word;
}

/* one get_field per field, as in hw01/status.c */
status_t status_unpack(uint32_t word) {
    status_t status;

    status.motor_open = get_field(word, STATUS_MOTOR_OPEN_POS, STATUS_FLAG_WIDTH);
    status.motor_close = get_field(word, STATUS_MOTOR_CLOSE_POS, STATUS_FLAG_WIDTH);
    status.wind = get_field(word, STATUS_WIND_POS, STATUS_FLAG_WIDTH);
    status.state = (state_t)get_field(word, STATUS_STATE_POS, STATUS_STATE_WIDTH);
    status.state_invalid = status.state >= N_STATES;    /* 3 bits hold 5..7 too */
    return status;
}
