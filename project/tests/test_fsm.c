/*
 * test_fsm.c: unit tests for fsm.h / fsm.c
 */
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

void test_fsm(void) {
    test_state_names();
    test_status_round_trip();
    test_status_examples();
    test_status_invalid();
}
