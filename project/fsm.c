#include "fsm.h"
#include "bits.h"       /* get_field, set_field */

static const char *state_names[N_STATES] = {
    [CLOSED]     = "CLOSED",
    [OPENING]    = "OPENING",
    [OPEN]       = "OPEN",
    [CLOSING]    = "CLOSING",
    [STORM_LOCK] = "STORM_LOCK",
};

/* name of state s, or "?" if out of range */
const char *state_name(state_t s) {
    if (s >= 0 && s < N_STATES) {
        return state_names[s];
    }
    return "?";
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
