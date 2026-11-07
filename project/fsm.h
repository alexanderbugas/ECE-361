/*
 * fsm: the vent's state machine. States: CLOSED, OPENING, OPEN, CLOSING,
 * and STORM_LOCK. A transition table, indexed by state and event, gives the
 * next state and action. Events come from the readings (too hot or humid,
 * cool and dry enough), from the motor finishing after 20 ticks, and from
 * the WIND_ALARM switch. See SPEC.md.
 */
#ifndef FSM_H
#define FSM_H

#include <stdint.h>     /* uint32_t, for the status word */
#include <time.h>       /* time_t, for the timers */
#include "sensor.h"     /* reading_t */

/* the five states given by the project sheet; N_STATES counts them */
typedef enum { CLOSED, OPENING, OPEN, CLOSING, STORM_LOCK, N_STATES } state_t;

/* the events (SPEC.md, "Events"); N_EVENTS counts them */
typedef enum {
    E_COOL_ENABLE,          /* at or above 28.0 C */
    E_HUMID_TRIGGER_HI,     /* at or above 85 %RH */
    E_COOL_DRY,             /* at or below 24.0 C and at or below 75 %RH */
    E_MOTOR_OPEN_20,        /* OPENING, and the motor has run 20 ticks */
    E_MOTOR_CLOSE_20,       /* CLOSING, and the motor has run 20 ticks */
    E_WIND_ALARM_ON,        /* the alarm is on, or a storm is not locked yet */
    E_WIND_ALARM_OFF_120,   /* STORM_LOCK, and the alarm off for 120 ticks */
    N_EVENTS
} event_t;

/* the actions: markers for the trace and the history stack. A_UNDEFINED is 0,
   so a cell left out of the table can be found (as in HW5). */
typedef enum {
    A_UNDEFINED, A_NONE, A_START_OPEN, A_START_CLOSE, A_STOP, A_STORM, A_STORM_CLEAR,
    N_ACTIONS
} action_t;

/* one cell of the transition table */
typedef struct { state_t next; action_t action; } transition_t;

/* the transition table: fsm_table[state][event] = { next state, action } */
extern const transition_t fsm_table[N_STATES][N_EVENTS];

/*
 * state_name, event_name, action_name: names for the trace
 * Returns: the name without its E_ or A_ prefix, or "?" if out of range
 */
const char *state_name(state_t s);
const char *event_name(event_t e);
const char *action_name(action_t a);

/*
 * fsm_step: looks up fsm_table[s][e]
 * Takes:   s, the current state; e, an event; action, where to write the
 *          action
 * Returns: the next state, with the action in *action. If s or e is out of
 *          range: s, with A_UNDEFINED in *action.
 * Assumes: action is not NULL
 */
state_t fsm_step(state_t s, event_t e, action_t *action);

/* what the event code remembers between ticks (SPEC.md, "Events") */
typedef struct {
    time_t motor_start;     /* when the state last changed into OPENING or CLOSING */
    time_t alarm_off_since; /* when the alarm last turned off */
    int alarm_was_on;       /* the alarm bit on the tick before */
    int storm_pending;      /* 1 from an alarm until the vent is locked */
} fsm_timers_t;

/*
 * fsm_timers_init: a fresh set of timers for a run that starts at now
 * Assumes: tm is not NULL
 */
void fsm_timers_init(fsm_timers_t *tm, time_t now);

/*
 * fsm_event: the one event for this tick, chosen by priority (motor, wind,
 * climate), or -1 if there is none. Reads the timers, changes nothing.
 * Takes:   tm, the timers; s, the current state; r, this tick's reading;
 *          switches, this tick's SWITCHES_REG (bit 0 is WIND_ALARM)
 * Returns: an event_t, or -1
 * Assumes: tm and r are not NULL, and fsm_tick keeps tm up to date
 */
int fsm_event(const fsm_timers_t *tm, state_t s, const reading_t *r, uint32_t switches);

/*
 * fsm_tick: one tick of the device: updates the alarm memory, gets the event,
 * steps the state machine, and starts the motor timer on the tick the state
 * changes into OPENING or CLOSING
 * Takes:   tm, the timers; s, the current state; r, this tick's reading;
 *          switches, this tick's SWITCHES_REG; event, where to write the event
 *          (or -1), or NULL; action, where to write the action
 * Returns: the next state. With no event: s, and A_NONE in *action.
 * Assumes: tm, r and action are not NULL
 */
state_t fsm_tick(fsm_timers_t *tm, state_t s, const reading_t *r, uint32_t switches,
                 int *event, action_t *action);

/* the outputs, set from the state every tick (SPEC.md, "Inputs and outputs") */
#define LED_MOTOR_OPEN   0x1u       /* bit 0 of LEDS_REG */
#define LED_MOTOR_CLOSE  0x2u       /* bit 1 of LEDS_REG */

/* one RGB color: the duty cycles build_rgb_reg takes, 0 to 255 */
typedef struct { uint8_t red; uint8_t green; uint8_t blue; } rgb_t;

/* the color of each state (SPEC.md): CLOSED green, OPENING orange, OPEN red,
   CLOSING yellow, STORM_LOCK black */
extern const rgb_t fsm_rgb[N_STATES];

/*
 * fsm_leds: the LEDS_REG value for a state
 * Returns: LED_MOTOR_OPEN while OPENING, LED_MOTOR_CLOSE while CLOSING,
 *          0 in every other state (and for a state out of range)
 */
uint32_t fsm_leds(state_t s);

/* status word field positions and widths (SPEC.md, "Status word") */
#define STATUS_FLAG_WIDTH       1
#define STATUS_MOTOR_OPEN_POS   0       /* same bit as MOTOR_OPEN in LEDS_REG */
#define STATUS_MOTOR_CLOSE_POS  1       /* same bit as MOTOR_CLOSE in LEDS_REG */
#define STATUS_WIND_POS         2
#define STATUS_STATE_POS        4
#define STATUS_STATE_WIDTH      3

/* the status word, one member per field */
typedef struct {
    state_t state;
    int motor_open;         /* 1 if MOTOR_OPEN is on */
    int motor_close;        /* 1 if MOTOR_CLOSE is on */
    int wind;               /* 1 if the wind alarm is on */
    int state_invalid;      /* 1 if the state field holds 5, 6 or 7 */
} status_t;

/*
 * status_pack: builds a status word with HW1's set_field
 * Takes:   s, the state; motor_open, motor_close, wind: 0 for off, any
 *          other value for on
 * Returns: the word; every bit outside the four fields is 0
 * Assumes: s is a state from 0 to N_STATES - 1
 */
uint32_t status_pack(state_t s, int motor_open, int motor_close, int wind);

/*
 * status_unpack: splits a status word into its fields with HW1's get_field
 * Takes:   word, a status word
 * Returns: the fields; state_invalid is 1 if the state field is not a state,
 *          and state then holds that raw value
 */
status_t status_unpack(uint32_t word);

#endif /* FSM_H */
