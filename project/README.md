# Project 18: Greenhouse vent

*Drafted by Claude at the user's request, from the planning notes the user
and Claude worked through in `MODULES.md`. The device itself is specified in
[SPEC.md](SPEC.md); this file records how the program is put together, where
each part comes from, and who decided what. Progress, module by module, is in
[PROGRESS.md](PROGRESS.md).*

## How to build and test

Run these from `project/`:

```bash
make            # builds vent with gcc -std=c11 -Wall -Wextra -g
make test       # the unit tests (with the sanitizers), then every scenario
make clean      # removes everything make built
./vent scenarios/NAME.txt   # replays a scenario: one trace line per tick
```

A trace line looks like the sheet's example:
`2026-10-06 08:00:46 state=OPENING temp=26.0 rh=70.0 leds=0x1 status=0x00000011`.
`--report` is accepted but not written yet (week 9): it says so and exits 1.

*The test setup was written by Claude at the user's request.*

- `tests/test_all.c` runs every module's tests and prints one line at the
  end: the number passed and failed. It exits with 1 if any failed, so
  `make test` fails.
- Each module has its own test file, `tests/test_<module>.c`, with one
  function that `test_all.c` calls.
- `tests/check.c` holds the `check(cond, name)` helper from HW5's tests: it
  prints PASS or FAIL with the test's name.
- The tests are built straight from the sources with
  `-fsanitize=address,undefined`, as the sheet requires, so they never share
  an object file with `vent`.
- `regress.sh` runs the unit tests, then `./vent` on every scenario in
  `scenarios/`, comparing each trace with the file of the same name in
  `expected/` (as in HW2 and HW6). It ends with the one line the sheet asks
  for: the number passed and failed, unit tests and scenarios together.

### The scenarios

*Written by Claude. Each expected trace was checked against states worked
out by hand from the rules before it was saved.*

| Scenario           | The sheet's | What it shows                                                                                                                                      |
| ------------------ | ----------- | -------------------------------------------------------------------------------------------------------------------------------------------------- |
| `1_normal.txt`     | 1           | warms to 28.0 C and opens (OPEN 20 ticks later), cools to 24.0 C and closes                                                                        |
| `3_thresholds.txt` | 3           | each threshold at exactly its value triggers; 27.9 C, 24.1 C, 75.1 %RH and 84.9 %RH do not                                                         |
| `4_storm.txt`      | 4           | a storm closes the open vent and locks it; the alarm comes back within 120 ticks, so the count starts over; the lock ends and the vent opens again |
| `5_unexpected.txt` | 5           | the switch changes in every state, including a gust that ends before the storm closing is done (the vent still locks)                              |
| `7_midnight.txt`   | 7           | starts at 23:59:40; the date changes at tick 20 while the vent is opening                                                                          |

Scenarios 2 (the stack grows, week 7) and 6 (the report, week 9) come later.

## How the program fits together

Once per tick, `main` runs this loop (from the project sheet, "main.c: the
program"):

```
           ┌────────────── main.c (scenario replay, clock, trace) ──────────────┐
           │                                                                    │
 scenario ─┼─► _iom361_set_sensor1 / _set_switches   (the "world", main only)   │
  line     │                                                                    │
           │   sensor_read(base, now, &reading) ──► reading_t {t, temp_c, rh}   │
           │                     │                                              │
           │   read SWITCHES_REG ┤                                              │
           │                     ▼                                              │
           │   event code: reading + switches + timers ──► one event, or -1     │
           │                     ▼                                              │
           │   fsm_step(state, event, &action) ──► next state + action          │
           │                     ▼                                              │
           │   write LEDS_REG and RGB_LED_REG from the state                    │
           │   status word = pack(state, motor bits, wind bit)                  │
           │   store_insert(store, record_t {reading, state, status})           │
           │   push onto the history stack (aux, week 7)                        │
           │   print one trace line                                             │
           └────────────────────────────────────────────────────────────────────┘
 after the run: the report (week 9) or the last 5 events from the stack
```

Only `main.c` and the tests call `_iom361_set_sensor1` and
`_iom361_set_switches`. `sensor.c` and `fsm.c` never do: on a real board
those functions would not exist.

### The modules, and the homework each one is built from

| Group       | Files                                                         | Job                                   | Built from                                     |
| ----------- | ------------------------------------------------------------- | ------------------------------------- | ---------------------------------------------- |
| **Measure** | `sensor.[ch]`, `iom361_r4.[ch]`                               | registers → `reading_t`               | HW6 Part 1 (`sensor_read`), HW3 contracts      |
| **Decide**  | `fsm.[ch]`                                                    | events → next state and action        | HW5 lock                                       |
| **Record**  | `store.[ch]`, `bits.[ch]`, the status word                    | keep every `record_t`, ordered by key | HW4 tally, HW1 `get_field`/`set_field`         |
| **History** | `aux.[ch]`                                                    | stack of vent events (week 7)         | HW7                                            |
| **Report**  | `report.[ch]`                                                 | top 5 temperatures (week 9)           | HW2 stats, HW3 `minmax`                        |
| **Program** | `main.c`, `scenario.[ch]`, `Makefile`, `tests/`, `scenarios/` | replay, clock, trace, tests           | HW6 Part 3 and 4, HW2 `regress.sh`, HW5 `main` |

The pieces each module reuses:

- **`bits.[ch]`** is copied unchanged from HW1:
  [hw01/bits.c:22-48](../hw01/bits.c#L22-L48). The status word is packed with
  `set_field` and unpacked with `get_field`. `set_field` returns 0 for a bad
  position or width ([hw01/bits.c:41-43](../hw01/bits.c#L41-L43)), which
  would erase the whole word, so the positions are constants, checked by the
  pack/unpack test.
- **The status word** follows HW1's status module: one `#define` per position
  and width ([hw01/status.h:5-15](../hw01/status.h#L5-L15)), and one
  `get_field` per field on unpack ([hw01/status.c:7-24](../hw01/status.c#L7-L24)).
  It differs in two ways: the word is `uint32_t`, not `uint16_t`, and there is
  also a pack function. HW1's `mode_invalid` check
  ([hw01/status.c:18-22](../hw01/status.c#L18-L22)) is the model for flagging
  a state value above 4.
- **`sensor_read`** follows the HW3 contract pattern of `minmax`
  ([hw03/ptr.h:14-26](../hw03/ptr.h#L14-L26)): check the arguments first,
  write the outputs only at the end, and leave them unchanged on failure
  ([hw03/ptr.c:23-40](../hw03/ptr.c#L23-L40)). The header comments use HW3's
  `Takes / Returns / Assumes` style.
- **The store** is an opaque type like HW4's tally:
  `typedef struct store store_t;` as in [hw04/tally.h:10](../hw04/tally.h#L10),
  with the inside written only in `store.c`, as in
  [hw04/tally.c:22-27](../hw04/tally.c#L22-L27). The sorted insert is
  `tally_add`'s: find the spot with a compare, shift the rest up one, write
  ([hw04/tally.c:68-96](../hw04/tally.c#L68-L96)). `record_compare` takes the
  place of `strcmp`. A new record goes **after** the records with an equal
  key, so equal keys stay in tick order. The `NULL` handling of tally's
  contracts ([hw04/tally.h:22-56](../hw04/tally.h#L22-L56)) is the model for
  the store's. `store.h` must not mention an index or an array, so that the
  tree in week 8 changes nothing outside `store.c`.
- **The state machine** has the shape of HW5's lock: the enums with an `N_`
  count last and `A_UNDEFINED = 0` ([hw05/lock.h:4-7](../hw05/lock.h#L4-L7)),
  a table with designated initializers ([hw05/lock.c:24-49](../hw05/lock.c#L24-L49)),
  a step function that only looks up the table
  ([hw05/lock.c:52-56](../hw05/lock.c#L52-L56)), and name functions for the
  trace ([hw05/lock.c:79-85](../hw05/lock.c#L79-L85)). The HW5 bonus checker
  ([hw05/check.c:140-148](../hw05/check.c#L140-L148)) works on any table of
  `transition_t`, so it can check the vent table for left-out cells and states
  with no way out.
- **The tests** follow [hw05/tests/test_lock.c](../hw05/tests/test_lock.c):
  every cell against a second copy of the table written in the test
  ([test_lock.c:49-91](../hw05/tests/test_lock.c#L49-L91)), and short
  sequences through a `run` helper
  ([test_lock.c:154-166](../hw05/tests/test_lock.c#L154-L166)).
- **A test program cannot link `main.o`**, because both have a `main()`
  ([hw05/Makefile:26-27](../hw05/Makefile#L26-L27) links tests from other
  `.o` files). So anything that needs a unit test lives outside `main.c`.

## Decisions

Who decided each item, and the reason given. "Settled" means the project
sheet, a handout, a lecture slide or C itself determines the answer. Reasons
in quotes are the user's own words.

| Decision                                                         | Choice                                                                                                               | Who                                       | Reason given                                                                                                                                                                                                          |
| ---------------------------------------------------------------- | -------------------------------------------------------------------------------------------------------------------- | ----------------------------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Status word layout                                               | flags at bits 0–2, state at bits 4–6 ("layout C")                                                                    | the user                                  | "it matches our current available code base and will easily plug into the project repo"                                                                                                                               |
| Where the status word lives                                      | `fsm.c`, with the state machine                                                                                      | the user, with Claude agreeing            | the user thought it "makes the most sense"; Claude: the store only holds what it is given, the way it holds the reading and the state, and the state machine is what knows the state and the motor                    |
| Where the event code lives                                       | `fsm.c`, with the status word and the timers                                                                         | the user                                  | "keep them together ... as they have accompanying functionality", and "to keep main tidy"                                                                                                                             |
| Where the motor timer lives                                      | with the state machine, "for now at least"                                                                           | the user                                  | none given                                                                                                                                                                                                            |
| `sensor_read`'s prototype and `reading_t`                        | as in the HW6 handout                                                                                                | settled                                   | HW6 Part 1 gives them                                                                                                                                                                                                 |
| `sensor_read` return value                                       | 0, or the iom361 `rc`; `*r` unchanged on failure                                                                     | settled                                   | HW6 Part 1                                                                                                                                                                                                            |
| `sensor_read` with a `NULL` `r`                                  | not allowed: the contract says `r` must not be `NULL`                                                                | the user                                  | none given                                                                                                                                                                                                            |
| A raw-to-tenths helper next to `sensor_read`                     | no: `sensor_read` only                                                                                               | the user                                  | "do it like the homework"                                                                                                                                                                                             |
| Rounding to 0.1                                                  | integers only, from the raw value                                                                                    | the user                                  | none given                                                                                                                                                                                                            |
| The `(int)(x + 0.5)` cast                                        | not used                                                                                                             | settled                                   | rounds negatives wrong; week 6 slide 8                                                                                                                                                                                |
| Store key                                                        | `floorf(temp_c * 2)`: a boundary value starts the step above                                                         | the user                                  | "we round up like in scientific notation"                                                                                                                                                                             |
| `roundf` or `(int)` for the key                                  | not used                                                                                                             | settled                                   | `roundf` breaks the sheet's example (21.2 and 21.4 must share a key); `(int)` makes the step around 0 °C 1.0 °C wide                                                                                                  |
| Duplicate keys                                                   | every record kept, equal keys in tick order; list or tie-break chosen in week 8                                      | Claude's suggestion, accepted by the user | the user asked for a count "unless you have a convincing reason otherwise"; Claude's reason: a count keeps one record per key, so the sheet's "find every one of them" test cannot pass and the report loses readings |
| Fixed size or `malloc` for the store                             | fixed size, no `malloc`, until `malloc` is covered                                                                   | the user                                  | `malloc` has not been covered in class yet                                                                                                                                                                            |
| `store_height` before the tree                                   | returns −1; height tests in week 8                                                                                   | the user                                  | none given                                                                                                                                                                                                            |
| Closing event                                                    | one combined "cool and dry" event                                                                                    | settled                                   | one lookup sees one event, and the given states cannot remember half a condition                                                                                                                                      |
| Opening events                                                   | two, one per cause                                                                                                   | the user                                  | "for easier debugging"                                                                                                                                                                                                |
| Motor events                                                     | two, one per direction                                                                                               | the user                                  | "we need to be more explicit"                                                                                                                                                                                         |
| Wind events                                                      | `WIND_ALARM_ON` and `WIND_ALARM_OFF_120`, with the 120 ticks counted outside the table                               | the user                                  | agreed ("sounds good")                                                                                                                                                                                                |
| LEDs                                                             | set from the state                                                                                                   | the user                                  | none given                                                                                                                                                                                                            |
| Several conditions in one tick                                   | one event per tick, by priority                                                                                      | the user                                  | none given                                                                                                                                                                                                            |
| Priority order                                                   | motor events, then wind, then climate                                                                                | settled                                   | with wind before the motor, a storm-closing vent never sees the motor finish and never locks; wind before climate, by rule 3                                                                                          |
| A gust that ends during closing                                  | remembered until `STORM_LOCK`                                                                                        | settled                                   | rule 3; otherwise the vent ends `CLOSED`, never locked                                                                                                                                                                |
| Every tick or only on a change                                   | every tick (level), timers included                                                                                  | settled, and the user's choice            | rule 1 and the sheet's "level" switch; the user: "we dont want to lose information"                                                                                                                                   |
| Ticks with no event                                              | −1, and no step                                                                                                      | the user                                  | "like homework 5"                                                                                                                                                                                                     |
| The 20 ticks                                                     | `now - motor_start >= 20`                                                                                            | settled                                   | rule 2, and "durations are differences between timestamps"                                                                                                                                                            |
| Cold and humid                                                   | opens at ≥ 85 %RH at any temperature                                                                                 | settled                                   | both halves of rule 1; the user: "use the design choice that does not violate rules"                                                                                                                                  |
| Wind while opening or open                                       | to `CLOSING`, then `STORM_LOCK` when shut                                                                            | the user                                  | "closing takes 20 ticks, we dont go straight to storm_lock as that breaks rule 2"                                                                                                                                     |
| Wind while closing                                               | keep closing, lock when shut                                                                                         | the user                                  | follows from the previous row ("ok")                                                                                                                                                                                  |
| Alarm off and on within 120 ticks                                | the count starts over                                                                                                | settled                                   | rule 3, "has been off for 120 ticks"                                                                                                                                                                                  |
| Leaving `STORM_LOCK`                                             | to `CLOSED`                                                                                                          | settled                                   | the vent is shut and no longer locked                                                                                                                                                                                 |
| Wind while `CLOSED`                                              | to `STORM_LOCK`                                                                                                      | settled                                   | rule 3, "from any state"                                                                                                                                                                                              |
| Climate changes while `OPENING` or `CLOSING` (silent rules 1, 2) | finish the movement, then move back                                                                                  | the user                                  | none given                                                                                                                                                                                                            |
| RGB colors                                                       | `CLOSED` green, `OPENING` orange, `OPEN` red, `CLOSING` yellow; `STORM_LOCK` open                                    | the user                                  | none given; the 0–255 values are Claude's proposal                                                                                                                                                                    |
| How find by key returns several records                          | a callback called once per match, like the in-order walk                                                             | the user                                  | none given                                                                                                                                                                                                            |
| The store's maximum                                              | 5000 records; an insert into a full store returns an error                                                           | Claude's proposal, accepted by the user   | "we'll fix it later. no malloc yet"                                                                                                                                                                                   |
| Where the clock function lives                                   | `scenario.c`                                                                                                         | the user                                  | none given                                                                                                                                                                                                            |
| Event and action names                                           | `COOL_DRY`, `MOTOR_OPEN_20`, `MOTOR_CLOSE_20`; `A_START_OPEN`, `A_START_CLOSE`, `A_STOP`, `A_STORM`, `A_STORM_CLEAR` | proposed by Claude, approved by the user  | none given                                                                                                                                                                                                            |
| The timers and the storm memory                                  | a struct passed by pointer                                                                                           | the user                                  | none given                                                                                                                                                                                                            |
| Comparing with the thresholds                                    | whole tenths, as integers                                                                                            | the user                                  | none given                                                                                                                                                                                                            |
| Order of the work                                                | project steps 1–9 first; then HW6; then HW6's sensor, scenario and loop are copied into the project                  | the user                                  | steps 1–9 do not need HW6                                                                                                                                                                                             |
| One test program for all modules                                 | `tests/test_all.c` calls one function per module file                                                                | Claude                                    | the sheet asks `make test` to end with one passed/failed line                                                                                                                                                         |
| Flags in `status_pack`                                           | any nonzero value packs as 1                                                                                         | Claude                                    | `set_field` cuts a value to its field, so 2 would pack as 0                                                                                                                                                           |
| `status_t`                                                       | a struct with one member per field, and `state_invalid` for a state field of 5 to 7                                  | Claude                                    | the shape of HW1's `status_t` and its `mode_invalid`                                                                                                                                                                  |
| The store's key                                                  | computed from `temp_c` each time, not stored in the record                                                           | Claude                                    | `record_t` keeps exactly the three fields the sheet lists                                                                                                                                                             |
| `store_key`                                                      | public in `store.h`, so a caller can turn a temperature into the key that `store_find` takes                         | Claude                                    | `store_find` takes a key, not a temperature                                                                                                                                                                           |
| `store_find`'s return value                                      | the number of matches; a `NULL` visit only counts                                                                    | Claude                                    | none beyond convenience for tests and the report                                                                                                                                                                      |
| Stores at once                                                   | a fixed pool of 4 (`STORE_POOL`), as tally's `TALLY_MAX`                                                             | Claude                                    | no `malloc` yet                                                                                                                                                                                                       |
| One function per tick                                            | `fsm_tick`: alarm memory, event, step, then the motor timer                                                          | Claude                                    | the motor timer has to start on the tick of the step that changes the state; `fsm_step` stays the sheet's "state and event in, next state out"                                                                        |
| `fsm_step` with a value out of range                             | stays in the same state, with `A_UNDEFINED`                                                                          | Claude                                    | the table has no such cell, so reading it would go past the array                                                                                                                                                     |
| Both opening conditions at once                                  | `COOL_ENABLE` is passed on                                                                                           | Claude                                    | the two columns are the same, so only the name changes                                                                                                                                                                |
| Names in code                                                    | `E_` and `A_` prefixes, as in HW5; the trace prints the names without them                                           | Claude                                    | the HW5 convention                                                                                                                                                                                                    |
| `sensor.c`, `scenario.[ch]` and the loop                         | copied from `hw06/`                                                                                                  | the user's plan                           | the HW6 handout's "next step"                                                                                                                                                                                         |
| The LED and RGB mapping                                          | `fsm_leds` and the `fsm_rgb` table in `fsm.c`; `main` passes the colors to `build_rgb_reg`                           | Claude                                    | in `fsm.c` they can be unit-tested, and `fsm.c` still does not need iom361                                                                                                                                            |
| Writing the LEDs                                                 | a plain write of the whole `LEDS_REG`, not HW6's `panel_led`                                                         | Claude                                    | both LEDs belong to the vent and are set from the state every tick, so there are no other bits to keep                                                                                                                |
| The wind bit of the status word                                  | the `WIND_ALARM` switch itself, not the remembered storm                                                             | Claude                                    | the sheet's field is "wind alarm"                                                                                                                                                                                     |
| A full store                                                     | `main` warns once on `stderr` and goes on; the trace is not affected                                                 | Claude                                    | the user: "we'll fix it later. no malloc yet"                                                                                                                                                                         |
| `sensor_read` failing in `vent`                                  | stop, as HW6's H4                                                                                                    | Claude, carried over from HW6             | the user chose it for HW6                                                                                                                                                                                             |

## Open questions

None at the moment. What `SPEC.md` still needs is listed at its end
("Still to come").
