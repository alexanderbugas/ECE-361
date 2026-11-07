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

/* the five states given by the project sheet; N_STATES counts them */
typedef enum { CLOSED, OPENING, OPEN, CLOSING, STORM_LOCK, N_STATES } state_t;

/*
 * state_name: the name of a state, for the trace
 * Takes:   s, a state
 * Returns: "CLOSED", "OPENING", ..., or "?" if s is out of range
 */
const char *state_name(state_t s);

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
