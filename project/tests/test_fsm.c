/*
 * test_fsm.c: unit tests for fsm.h / fsm.c
 */
#include <stdio.h>
#include <string.h>
#include "check.h"
#include "fsm.h"

/*
 * The names, and the values the status word and the trace rely on
 *
 * Why: CLOSED must be 0 (the initial state, and an all-zero status word), and
 * the trace prints these exact names.
 */
static void test_state_names(void) {
    check(CLOSED == 0 && STORM_LOCK == 4 && N_STATES == 5, "fsm: states are 0 to 4");
    check(strcmp(state_name(CLOSED), "CLOSED") == 0, "fsm: name of CLOSED");
    check(strcmp(state_name(OPENING), "OPENING") == 0, "fsm: name of OPENING");
    check(strcmp(state_name(OPEN), "OPEN") == 0, "fsm: name of OPEN");
    check(strcmp(state_name(CLOSING), "CLOSING") == 0, "fsm: name of CLOSING");
    check(strcmp(state_name(STORM_LOCK), "STORM_LOCK") == 0, "fsm: name of STORM_LOCK");
    check(strcmp(state_name((state_t)7), "?") == 0, "fsm: name of state 7 is ?");
    check(strcmp(state_name((state_t)-1), "?") == 0, "fsm: name of state -1 is ?");
}

/*
 * Pack then unpack gives back every field, for every state and every flag
 * combination, and no bit outside the four fields is ever set
 *
 * Why: this is the sheet's required status-word test.
 */
static void test_status_round_trip(void) {
    int all_back = 1;
    int unused_zero = 1;
    uint32_t used = 0x77u;      /* bits 0-2 and 4-6 */

    for (int s = 0; s < N_STATES; s++) {
        for (int flags = 0; flags < 8; flags++) {
            int mo = flags & 1, mc = (flags >> 1) & 1, w = (flags >> 2) & 1;
            uint32_t word = status_pack((state_t)s, mo, mc, w);
            status_t st = status_unpack(word);

            if (st.state != (state_t)s || st.motor_open != mo ||
                st.motor_close != mc || st.wind != w || st.state_invalid) {
                all_back = 0;
            }
            if (word & ~used) {
                unused_zero = 0;
            }
        }
    }
    check(all_back, "status: pack then unpack gives back every field (40 words)");
    check(unused_zero, "status: unused bits are 0 (40 words)");
}

/*
 * The example values in SPEC.md, and the sheet's example trace line
 *
 * Why: these are the values a trace shows, so they are checked exactly.
 */
static void test_status_examples(void) {
    check(status_pack(CLOSED, 0, 0, 0) == 0x00u, "status: CLOSED is 0x00");
    check(status_pack(OPENING, 1, 0, 0) == 0x11u, "status: OPENING, motor open is 0x11 (the sheet's example)");
    check(status_pack(OPEN, 0, 0, 0) == 0x20u, "status: OPEN is 0x20");
    check(status_pack(CLOSING, 0, 1, 0) == 0x32u, "status: CLOSING, motor close is 0x32");
    check(status_pack(STORM_LOCK, 0, 0, 1) == 0x44u, "status: STORM_LOCK, wind on is 0x44");
    check(status_pack(OPEN, 2, 0, 5) == 0x25u, "status: any nonzero flag packs as 1");
}

/*
 * A state field of 5, 6 or 7 is flagged, as HW1's mode_invalid was
 *
 * Why: 3 bits hold 0 to 7, but there are only 5 states.
 */
static void test_status_invalid(void) {
    status_t st = status_unpack(0x70u);         /* state field 7 */

    check(st.state_invalid == 1, "status: a state field of 7 is invalid");
    check(status_unpack(0x50u).state_invalid == 1, "status: a state field of 5 is invalid");
    check(status_unpack(0x40u).state_invalid == 0, "status: a state field of 4 is valid");
}

/*
 * Every one of the 35 cells: fsm_step gives the expected next state and
 * action
 *
 * Why: the expected values are written out here, separately from fsm_table,
 * so a typo in fsm.c shows up as a mismatch instead of being copied (HW5).
 */
static void test_table_cells(void) {
    static const transition_t expected[N_STATES][N_EVENTS] = {
        [CLOSED] = {
            [E_COOL_ENABLE] = { OPENING, A_START_OPEN },   [E_HUMID_TRIGGER_HI] = { OPENING, A_START_OPEN },
            [E_COOL_DRY]    = { CLOSED, A_NONE },          [E_MOTOR_OPEN_20]    = { CLOSED, A_NONE },
            [E_MOTOR_CLOSE_20] = { CLOSED, A_NONE },       [E_WIND_ALARM_ON]    = { STORM_LOCK, A_STORM },
            [E_WIND_ALARM_OFF_120] = { CLOSED, A_NONE },
        },
        [OPENING] = {
            [E_COOL_ENABLE] = { OPENING, A_NONE },         [E_HUMID_TRIGGER_HI] = { OPENING, A_NONE },
            [E_COOL_DRY]    = { OPENING, A_NONE },         [E_MOTOR_OPEN_20]    = { OPEN, A_STOP },
            [E_MOTOR_CLOSE_20] = { OPENING, A_NONE },      [E_WIND_ALARM_ON]    = { CLOSING, A_START_CLOSE },
            [E_WIND_ALARM_OFF_120] = { OPENING, A_NONE },
        },
        [OPEN] = {
            [E_COOL_ENABLE] = { OPEN, A_NONE },            [E_HUMID_TRIGGER_HI] = { OPEN, A_NONE },
            [E_COOL_DRY]    = { CLOSING, A_START_CLOSE },  [E_MOTOR_OPEN_20]    = { OPEN, A_NONE },
            [E_MOTOR_CLOSE_20] = { OPEN, A_NONE },         [E_WIND_ALARM_ON]    = { CLOSING, A_START_CLOSE },
            [E_WIND_ALARM_OFF_120] = { OPEN, A_NONE },
        },
        [CLOSING] = {
            [E_COOL_ENABLE] = { CLOSING, A_NONE },         [E_HUMID_TRIGGER_HI] = { CLOSING, A_NONE },
            [E_COOL_DRY]    = { CLOSING, A_NONE },         [E_MOTOR_OPEN_20]    = { CLOSING, A_NONE },
            [E_MOTOR_CLOSE_20] = { CLOSED, A_STOP },       [E_WIND_ALARM_ON]    = { CLOSING, A_NONE },
            [E_WIND_ALARM_OFF_120] = { CLOSING, A_NONE },
        },
        [STORM_LOCK] = {
            [E_COOL_ENABLE] = { STORM_LOCK, A_NONE },      [E_HUMID_TRIGGER_HI] = { STORM_LOCK, A_NONE },
            [E_COOL_DRY]    = { STORM_LOCK, A_NONE },      [E_MOTOR_OPEN_20]    = { STORM_LOCK, A_NONE },
            [E_MOTOR_CLOSE_20] = { STORM_LOCK, A_NONE },   [E_WIND_ALARM_ON]    = { STORM_LOCK, A_NONE },
            [E_WIND_ALARM_OFF_120] = { CLOSED, A_STORM_CLEAR },
        },
    };

    for (int s = 0; s < N_STATES; s++) {
        for (int e = 0; e < N_EVENTS; e++) {
            action_t action;
            state_t next = fsm_step((state_t)s, (event_t)e, &action);
            char name[96];

            snprintf(name, sizeof name, "table: %s %s -> %s %s",
                     state_name((state_t)s), event_name((event_t)e),
                     state_name(expected[s][e].next), action_name(expected[s][e].action));
            check(next == expected[s][e].next && action == expected[s][e].action, name);
        }
    }

    action_t action;
    check(fsm_step((state_t)7, E_COOL_DRY, &action) == (state_t)7 && action == A_UNDEFINED,
          "table: a state out of range stays, with A_UNDEFINED");
    check(fsm_step(OPEN, (event_t)9, &action) == OPEN && action == A_UNDEFINED,
          "table: an event out of range stays, with A_UNDEFINED");
    check(strcmp(event_name(E_COOL_DRY), "COOL_DRY") == 0 && strcmp(action_name(A_STORM_CLEAR), "STORM_CLEAR") == 0
          && strcmp(event_name((event_t)-1), "?") == 0, "fsm: event and action names");
}

/* ------------------------------------------------------------------ */
/* The event code, tick by tick: a tiny replay of the "world"          */

static fsm_timers_t tm;
static state_t state;

/* starts a run at tick 0 in state s */
static void start(state_t s) {
    fsm_timers_init(&tm, 0);
    state = s;
}

/* runs ticks from..to (inclusive) with one temperature, humidity and switch
   value, and returns the state after the last one */
static state_t run(int from, int to, float temp_c, float rh_pct, uint32_t switches) {
    for (int t = from; t <= to; t++) {
        reading_t r = { .t = t, .temp_c = temp_c, .rh_pct = rh_pct };
        action_t action;
        state = fsm_tick(&tm, state, &r, switches, NULL, &action);
    }
    return state;
}

/* the event one tick would give, without running it */
static int event_at(state_t s, float temp_c, float rh_pct, uint32_t switches) {
    fsm_timers_t fresh;
    reading_t r = { .t = 0, .temp_c = temp_c, .rh_pct = rh_pct };

    fsm_timers_init(&fresh, 0);
    return fsm_event(&fresh, s, &r, switches);
}

/*
 * Every threshold exactly at its value, and just past it
 *
 * Why: the rules say "at or above" and "at or below" (SPEC.md).
 */
static void test_thresholds(void) {
    check(event_at(CLOSED, 28.0f, 50.0f, 0) == E_COOL_ENABLE, "event: 28.0 C is too hot");
    check(event_at(CLOSED, 27.9f, 50.0f, 0) == -1, "event: 27.9 C, 50 %RH is no event (the gap)");
    check(event_at(CLOSED, 20.0f, 85.0f, 0) == E_HUMID_TRIGGER_HI, "event: 85.0 %RH is too humid");
    check(event_at(CLOSED, 20.0f, 84.9f, 0) == -1, "event: 20.0 C, 84.9 %RH is no event");
    check(event_at(OPEN, 24.0f, 75.0f, 0) == E_COOL_DRY, "event: 24.0 C and 75.0 %RH is cool and dry");
    check(event_at(OPEN, 24.1f, 75.0f, 0) == -1, "event: 24.1 C is not cool enough");
    check(event_at(OPEN, 24.0f, 75.1f, 0) == -1, "event: 75.1 %RH is not dry enough");
    check(event_at(OPEN, -12.3f, 50.0f, 0) == E_COOL_DRY, "event: -12.3 C is cool (tenths below 0)");
    check(event_at(CLOSED, 15.0f, 90.0f, 0) == E_HUMID_TRIGGER_HI, "event: cold and humid opens (silent rule 3)");
    check(event_at(CLOSED, 30.0f, 50.0f, 0x1u) == E_WIND_ALARM_ON, "event: wind comes before heat");
}

/*
 * Opening takes exactly 20 ticks; the motor event comes before the climate
 *
 * Why: rule 2, and silent rule 1 (finish opening, then close).
 */
static void test_motor(void) {
    start(CLOSED);
    check(run(0, 0, 28.0f, 50.0f, 0) == OPENING, "motor: too hot on tick 0 starts opening");
    check(run(1, 19, 28.0f, 50.0f, 0) == OPENING, "motor: still OPENING on tick 19 (heat does not restart it)");
    check(run(20, 20, 28.0f, 50.0f, 0) == OPEN, "motor: OPEN on tick 20, 20 ticks after the start");

    start(CLOSED);
    run(0, 0, 28.0f, 50.0f, 0);                 /* opening from tick 0 */
    check(run(5, 19, 24.0f, 70.0f, 0) == OPENING, "silent rule 1: cool and dry while opening keeps OPENING");
    check(run(20, 20, 24.0f, 70.0f, 0) == OPEN, "silent rule 1: OPEN on tick 20 (motor before climate)");
    check(run(21, 21, 24.0f, 70.0f, 0) == CLOSING, "silent rule 1: CLOSING on tick 21");
    check(run(22, 40, 24.0f, 70.0f, 0) == CLOSING && run(41, 41, 24.0f, 70.0f, 0) == CLOSED,
          "silent rule 1: CLOSED on tick 41");

    start(OPEN);
    run(0, 0, 24.0f, 70.0f, 0);                 /* closing from tick 0 */
    check(run(5, 19, 30.0f, 50.0f, 0) == CLOSING, "silent rule 2: too hot while closing keeps CLOSING");
    check(run(20, 20, 30.0f, 50.0f, 0) == CLOSED && run(21, 21, 30.0f, 50.0f, 0) == OPENING,
          "silent rule 2: CLOSED on tick 20, OPENING on tick 21");
}

/*
 * The storm: close, then lock; a short gust still locks; the 120 ticks
 * start over when the alarm comes back
 *
 * Why: rule 3, silent rules 4 to 8, and the priority order.
 */
static void test_storm(void) {
    start(OPEN);
    check(run(0, 0, 30.0f, 50.0f, 0x1u) == CLOSING, "storm: wind while OPEN starts closing");
    check(run(1, 19, 30.0f, 50.0f, 0x1u) == CLOSING, "storm: still CLOSING on tick 19");
    check(run(20, 20, 30.0f, 50.0f, 0x1u) == CLOSED, "storm: CLOSED on tick 20");
    check(run(21, 21, 30.0f, 50.0f, 0x1u) == STORM_LOCK, "storm: STORM_LOCK on tick 21");
    check(run(22, 60, 30.0f, 50.0f, 0x1u) == STORM_LOCK, "storm: heat does not open a locked vent");
    check(run(61, 180, 30.0f, 50.0f, 0) == STORM_LOCK, "storm: still locked 119 ticks after the alarm went off");
    check(run(181, 181, 30.0f, 50.0f, 0) == CLOSED, "storm: CLOSED 120 ticks after the alarm went off");
    check(run(182, 182, 30.0f, 50.0f, 0) == OPENING, "storm: hot after the lock, so OPENING on the next tick");

    start(OPEN);
    run(0, 0, 25.0f, 70.0f, 0x1u);              /* a one-tick gust */
    check(run(1, 20, 25.0f, 70.0f, 0) == CLOSED, "gust: the vent still closes after the alarm went off");
    check(run(21, 21, 25.0f, 70.0f, 0) == STORM_LOCK, "gust: and it locks");
    check(run(22, 120, 25.0f, 70.0f, 0) == STORM_LOCK && run(121, 121, 25.0f, 70.0f, 0) == CLOSED,
          "gust: unlocked 120 ticks after the alarm went off (tick 1)");

    start(CLOSED);
    check(run(0, 9, 20.0f, 50.0f, 0x1u) == STORM_LOCK, "storm: wind while CLOSED locks at once");
    run(10, 49, 20.0f, 50.0f, 0);               /* off from tick 10 */
    run(50, 59, 20.0f, 50.0f, 0x1u);            /* on again within the 120 ticks */
    check(run(60, 179, 20.0f, 50.0f, 0) == STORM_LOCK, "storm: back on within 120 ticks: still locked at 179");
    check(run(180, 180, 20.0f, 50.0f, 0) == CLOSED, "storm: the count started over at tick 60");
}

/*
 * The outputs from the state: the motor LEDs and the RGB color (SPEC.md)
 */
static void test_outputs(void) {
    check(fsm_leds(OPENING) == 0x1u && fsm_leds(CLOSING) == 0x2u, "outputs: MOTOR_OPEN while OPENING, MOTOR_CLOSE while CLOSING");
    check(fsm_leds(CLOSED) == 0 && fsm_leds(OPEN) == 0 && fsm_leds(STORM_LOCK) == 0, "outputs: no motor LED in the other states");
    check(fsm_rgb[CLOSED].green == 255 && fsm_rgb[CLOSED].red == 0, "outputs: CLOSED is green");
    check(fsm_rgb[OPENING].red == 255 && fsm_rgb[OPENING].green == 128, "outputs: OPENING is orange");
    check(fsm_rgb[OPEN].red == 255 && fsm_rgb[OPEN].green == 0, "outputs: OPEN is red");
    check(fsm_rgb[CLOSING].red == 255 && fsm_rgb[CLOSING].green == 255, "outputs: CLOSING is yellow");
    check(fsm_rgb[STORM_LOCK].red == 0 && fsm_rgb[STORM_LOCK].green == 0 && fsm_rgb[STORM_LOCK].blue == 0,
          "outputs: STORM_LOCK is black");
    int distinct = 1;
    for (int a = 0; a < N_STATES; a++) {
        for (int b = a + 1; b < N_STATES; b++) {
            if (fsm_rgb[a].red == fsm_rgb[b].red && fsm_rgb[a].green == fsm_rgb[b].green && fsm_rgb[a].blue == fsm_rgb[b].blue) {
                distinct = 0;
            }
        }
    }
    check(distinct, "outputs: the five colors are all different (the sheet)");
}

void test_fsm(void) {
    test_outputs();
    test_state_names();
    test_status_round_trip();
    test_status_examples();
    test_status_invalid();
    test_table_cells();
    test_thresholds();
    test_motor();
    test_storm();
}
