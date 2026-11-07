# Progress

*Kept by Claude. One row per module. A step is marked done only when
`make test` passes with no warnings. Scope for now: the project through week 6
of the sheet's plan (Table 8). Weeks 7 to 10 are listed at the end, not
planned yet.*

*Order of the work (the user's plan): steps 1 to 9 now, then HW6 in `hw06/`
(its own tracker: [hw06/PROGRESS.md](../hw06/PROGRESS.md)), then steps 10 to
14, which copy HW6's `sensor.c`, scenario code and loop into the project.*

## Status

| #   | Module or file                                        | Week | Status       | Waits for             |
| --- | ----------------------------------------------------- | ---- | ------------ | --------------------- |
| —   | `ASSIGNMENT.md`, iom361 files, stubs                  | 2    | **complete** |                       |
| —   | `Makefile` (builds `vent`)                            | 2    | **complete** |                       |
| —   | `SPEC.md`, `README.md`                                | all  | in progress  | updated at every step |
| 1   | `tests/` and `make test`                              | 3    | **complete** |                       |
| 2   | `fsm.h`: states and `state_name`                      | 3    | **complete** |                       |
| 3   | `bits.[ch]`, copied from HW1                          | 3    | **complete** |                       |
| 4   | the status word: pack and unpack                      | 3    | **complete** |                       |
| 5   | `sensor.h`: `reading_t`, `sensor_read`                | 3, 4 | **complete** |                       |
| 6   | `store.h`: `record_t`, the interface                  | 3, 4 | **complete** |                       |
| 7   | `store.c`: sorted array                               | 4    | **complete** |                       |
| 8   | `fsm.h` / `fsm.c`: events, actions, table, `fsm_step` | 5    | **complete** |                       |
| 9   | the event code and its timers                         | 5    | **complete** |                       |
| 10  | `sensor.c`                                            | 6    | **complete** |                       |
| 11  | `scenario.[ch]`                                       | 6    | **complete** |                       |
| 12  | the clock, in `scenario.c`                            | 6    | **complete** |                       |
| 13  | `main.c`: replay, trace, LEDs, RGB                    | 6    | **complete** |                       |
| 14  | scenarios 1, 3, 4, 5, 7 and `regress.sh`              | 5, 6 | **complete** |                       |
| 15  | `AI_USAGE.md` for the project                         | all  | in progress  |                       |

## The plan, one step at a time

Each step is one module (or one header and its `.c`). For each step Claude
writes the code and its tests, runs `make test` and shows the result, explains
every change, and updates `SPEC.md`, `README.md` and this file. Nothing is
committed: the user commits after reading the diff. The steps are done in batches (1–4, 5–7, 8–9), with a review after each
batch.

1. **`tests/` and `make test`.** A test program per module with HW5's
   `check(cond, name)` helper ([hw05/tests/test_lock.c:32-40](../hw05/tests/test_lock.c#L32-L40)),
   PASS or FAIL per test, a nonzero exit on failure. `make test` builds and
   runs them all and ends with one line: the number passed and failed. The
   tests are built with `-fsanitize=address,undefined`, as the sheet
   requires. Each `.o` lists the headers it includes.
2. **`fsm.h`: the five states** in the sheet's order (`CLOSED` = 0), with
   `N_STATES` last, and `state_name` (with `"?"` for a value out of range).
   This comes first because the status word and `record_t` both hold a state.
   Tests: every name, and a value out of range.
3. **`bits.[ch]`**, copied unchanged from HW1. Tests: `get_field` and
   `set_field` on the status word's positions.
4. **The status word** (layout C in `SPEC.md`): the position `#define`s, a
   pack function and an unpack function, in `fsm.c` (the user's choice). Tests: pack then unpack gives back every field, for every
   state, and unused bits are 0; the examples in `SPEC.md` (`0x11`, `0x32`,
   `0x44`).
5. **`sensor.h`**: `reading_t` and the `sensor_read` prototype from HW6
   Part 1, with a `Takes / Returns / Assumes` contract.
6. **`store.h`**: `record_t`; `typedef struct store store_t;`; and the
   prototypes, each with its contract: `record_compare`, create, insert, find
   by key, the in-order walk with a callback
   (`void (*visit)(const record_t *r, void *ctx)`), count, height, destroy.
7. **`store.c`**: a fixed-size sorted array behind that interface, no
   `malloc`. Insert after equal keys, so equal keys stay in tick order.
   At most 5000 records. `find` calls a callback once per match.
   `store_height` returns −1. Tests from the sheet: `record_compare` in the
   same step (21.2 and 21.4 → 0), neighboring steps (21.4 and 21.6 → <0), on
   a boundary (21.5 against 21.4 → >0, and against 21.9 → 0), and one pair
   below 0 °C (−0.2 and 0.2 → different keys); insert into an empty store;
   insert duplicates and find every one; find a key that is not there; the
   walk visits records in key order; count after 1, 2 and 100 inserts;
   destroy an empty store; a full store refuses an insert. (Height tests:
   week 8.)
8. **`fsm.h` / `fsm.c`**: the events and actions, the transition table from
   `SPEC.md`, `fsm_step`, and the name functions, in HW5's shape. Tests:
   every one of the 35 cells against a second copy of the table written in
   the test, and short sequences. The HW5 bonus checker can be run on the
   table as an extra test (no left-out cells, every state reachable from
   `CLOSED`, no state without a way out).
9. **The event code**: reading + switch register + timers → one event, or
   −1, by priority, with the motor timer and the 120-tick storm count.
   Tests: each threshold exactly at its value; wind first on a hot, stormy
   tick; the motor finishing on exactly the 20th tick; the storm count
   starting over.
10. **`sensor.c`**: both reads, mask 19..0, integer rounding, `*r` written
    only at the end. Tests: 91.0 °C reads back as 91.0 (HW6); 0 °C and
    100 °C (the sheet); 28.0 °C counts as at or above (a threshold); a
    negative value; a failed read changes nothing.
11. **`scenario.[ch]`**: HW6 Part 3. The sheet's format (`#` comments, a
    `start <timestamp>` line, then `tick temp rh switches`, with hex
    switches and increasing ticks), at most 1000 lines, no `malloc`.
    `scenario_load` returns 0 or the number of the first wrong line, and a
    function gives the line in force at a tick. Tests from HW6: comments and
    blank lines, values holding between lines, and a file with no start
    line, with a missing field, and with ticks that do not increase.
12. **The clock**, in `scenario.c` (the user's choice): `now = start + tick`, printed in UTC with `gmtime` and
    `strftime` as `2026-10-06 08:00:46`. Tests from the sheet: the start time
    prints as given, 59 seconds later, one second after 23:59:59 is 00:00:00
    on the next day, and 28 February 2027 to 1 March.
13. **`main.c`**: `./vent scenario.txt`. It initializes iom361 with 1 switch
    and 2 LEDs, turns the display off, and for every tick from 0 to the last
    one: sets the world from the line in force, reads the sensor and the
    switches, gets the event, steps the state machine, writes the LEDs and
    the RGB LED from the state, packs the status word, stores the record,
    and prints one trace line in the sheet's format:
    `2026-10-06 08:00:46 state=OPENING temp=26.0 rh=70.0 leds=0x1 status=0x00000011`.
    `--report` comes in week 9.
14. **Scenario tests**: scenarios 1 (normal operation), 3 (every threshold
    exactly at its value), 4 (rule 3 happens and clears), 5 (every switch
    changes in every state) and 7 (midnight), each in `scenarios/` with its
    expected trace, compared by a `regress.sh` with `diff`, as in HW2. `make
    test` runs the unit tests, then the scenarios.
15. **`AI_USAGE.md`** for the project, updated with each step.

## Not planned yet (weeks 7 to 10)

| Module                        | Week | Note                                                         |
| ----------------------------- | ---- | ------------------------------------------------------------ |
| `aux.[ch]`: the history stack | 7    | starts with room for 8 and doubles: needs `malloc`/`realloc` |
| scenario 2 (the stack grows)  | 7    |                                                              |
| the tree inside `store.c`     | 8    | nodes need `malloc`, or a fixed pool; height tests           |
| duplicates in the tree        | 8    | README question 3                                            |
| `report.[ch]` and `--report`  | 9    |                                                              |
| scenario 6 (the report)       | 9    |                                                              |
| the rest of `SPEC.md`         | 10   | drawing, 2038, the two paragraphs                            |
