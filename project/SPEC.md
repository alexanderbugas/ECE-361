# Greenhouse vent: specification

Project 18, ECE 361, Fall 2026. This file holds everything the project sheet
asks `SPEC.md` to contain. Items marked **Open** are not decided yet.

## States

Given by the project sheet.

| State        | Value | Meaning                              |
| ------------ | ----- | ------------------------------------ |
| `CLOSED`     | 0     | vent shut (initial state)            |
| `OPENING`    | 1     | motor opening                        |
| `OPEN`       | 2     | vent fully open                      |
| `CLOSING`    | 3     | motor closing                        |
| `STORM_LOCK` | 4     | vent shut and locked because of wind |

## Inputs and outputs

- **Switch:** bit 0 of `SWITCHES_REG` is `WIND_ALARM`, a level switch: on
  means storm, for as long as it is on. There is no wind speed.
- **LEDs:** bit 0 of `LEDS_REG` is `MOTOR_OPEN` (lit while opening), bit 1 is
  `MOTOR_CLOSE` (lit while closing). Both are set **from the state** every
  tick: `OPENING` → `0x1`, `CLOSING` → `0x2`, any other state → `0x0`.
- **RGB LED:** one color per state, also set from the state with
  `build_rgb_reg(1, red, green, blue)`:

  | State        | Color    | Red, green, blue                     |
  | ------------ | -------- | ------------------------------------ |
  | `CLOSED`     | green    | 0, 255, 0                            |
  | `OPENING`    | orange   | 255, 128, 0                          |
  | `OPEN`       | red      | 255, 0, 0                            |
  | `CLOSING`    | yellow   | 255, 255, 0                          |
  | `STORM_LOCK` | black    | 0, 0, 0                              |

  The enable bit is always 1, so black is `0x80000000` in the register, not
  `0x00000000` ("off"), and iom361 prints it with `Enable=ON`. On a real LED,
  black gives no light, so it would look the same as an LED that is off.
- **Readings:** the device accepts every value the sensor can report: about
  −50 to 150 °C and 0 to 100 %RH.

## Readings

`reading_t` and `sensor_read` are the ones from HW6 Part 1:

```c
typedef struct { time_t t; float temp_c; float rh_pct; } reading_t;
int sensor_read(uint32_t *base, time_t now, reading_t *r);
```

- `sensor_read` reads `TEMP_REG` and `HUMID_REG`, keeps bits 19..0 of each,
  converts and rounds to 0.1, and stores the result with the timestamp `now`
  in `*r`. It returns 0, or the iom361 return code of the read that failed,
  and then it changes nothing in `*r`.
- **Rounding is done with integers only**, straight from the raw value (week
  6 lecture, slides 8 to 10):
  - temperature in tenths: `(int32_t)((st * 2000u + (1u << 19)) >> 20) - 500`
  - humidity in tenths: `(srh * 1000u + (1u << 19)) >> 20`

  Adding `1u << 19`, half of 2^20, makes the shift round instead of cut off.
  The largest raw value gives 2,097,674,288 before the shift, which fits in
  32 bits. The tenths are stored as `tenths / 10.0f`.
- Checked against iom361: every temperature from −50.0 to 149.9 °C and every
  humidity from 0.0 to 100.0 %RH, set with `_iom361_set_sensor1`, reads back
  as the value that was set.
- `r` must not be `NULL`. `sensor_read`'s contract in `sensor.h` says so; the
  function does not check it.

## Events

An event is produced from the reading, the switch register and the timers.

| Event                | Happens when                                                                      | Rule |
| -------------------- | --------------------------------------------------------------------------------- | ---- |
| `COOL_ENABLE`        | the temperature is at or above 28.0 °C (too hot)                                  | 1    |
| `HUMID_TRIGGER_HI`   | the humidity is at or above 85 %RH (too humid)                                    | 1    |
| `COOL_DRY`           | the temperature is at or below 24.0 °C **and** the humidity is at or below 75 %RH | 1    |
| `MOTOR_OPEN_20`      | the state is `OPENING` and the motor has run for 20 ticks                         | 2    |
| `MOTOR_CLOSE_20`     | the state is `CLOSING` and the motor has run for 20 ticks                         | 2    |
| `WIND_ALARM_ON`      | the `WIND_ALARM` switch is on                                                     | 3    |
| `WIND_ALARM_OFF_120` | the state is `STORM_LOCK` and the alarm has been off for 120 ticks                | 3    |

How the events are produced:

- **"Cool and dry" is one event.** The vent closes only when both conditions
  hold. A table lookup sees one event, and the five given states cannot
  remember that half of the condition already happened, so the two
  conditions are checked together and give one event. The first version of
  this file had them as two events, `COOL_LIMIT` and `HUMID_TRIGGER_LOW`.
- **Opening has two events,** one per cause, so that a test or a debugger can
  see which one fired. Their table columns are the same.
- **The motor has two events,** one per direction, to be explicit.
- **Every event is produced on every tick its condition holds** ("level"),
  not only on the tick it becomes true. Rule 1 is about the current reading:
  at 28.0 °C or above, the vent should be open or opening. If events fired
  only on a change, a vent that leaves `STORM_LOCK` during a hot spell would
  stay `CLOSED` at 30 °C, because "too hot" became true before the storm and
  has not changed since. The sheet also calls `WIND_ALARM` a level switch.
  The timer events are level too: they fire on every tick after the time is
  up, for as long as the state is still waiting for them.
- **One event per tick, chosen by priority.** When more than one condition
  holds, only the most important one is passed to the state machine. Because
  every event is level, a condition that loses on one tick is produced again
  on the next, so nothing is lost: it is at most a tick late.
  The order is **the motor events, then the wind events, then the climate
  events**:
  - **The motor events first.** On a stormy tick the vent is `CLOSING`
    (silent rule 4), and the alarm stays on. If `WIND_ALARM_ON` came before
    `MOTOR_CLOSE_20`, it would win every tick, the motor finishing would never
    be seen, and the vent would stay `CLOSING` and never lock. With the motor
    first, the vent is `CLOSED` on the tick the closing ends and `STORM_LOCK`
    on the next. (Checked with a small simulation.) The motor events never
    start a movement, they only end one, so putting them first never opens
    the vent in a storm.
  - **The wind events next, before the climate events.** Rule 3 says the
    alarm closes the vent from any state, so on a hot, stormy tick the vent
    must not start opening.
  - **When both opening conditions hold** (28.0 °C or more and 85 %RH or
    more), `COOL_ENABLE` is the one passed on. Both columns have the same
    cells, so it changes nothing but the name.
  - **The climate events never compete with each other.** "Cool and dry" can
    never hold on the same tick as either opening event (28.0 and 24.0 °C,
    and 85 and 75 %RH, cannot both be met). The two motor events cannot both
    hold, because each needs a different state. The two wind events cannot
    either: one needs the alarm on (or a storm not yet locked), the other
    needs the vent locked and the alarm off for 120 ticks.
- **A tick with no event** gives −1, and the state machine is not run on that
  tick, as in HW5.
- **Thresholds are compared in whole tenths,** as integers: 280 (28.0 °C),
  240 (24.0 °C), 850 (85 %RH) and 750 (75 %RH). `reading_t` holds floats, as
  HW6 requires, so the event code turns `temp_c` and `rh_pct` back into
  tenths by rounding half away from zero:
  `(int)(x * 10.0f + (x >= 0.0f ? 0.5f : -0.5f))`. This is exact: every tenth
  from −50.0 to 150.0, stored as `tenths / 10.0f`, comes back as the same
  tenths (checked).
- **Thresholds are inclusive,** as the rules say "at or above" and "at or
  below": 28.0 °C opens, and 24.0 °C (with ≤ 75 %RH) closes. Between the
  thresholds no climate event happens and the vent stays as it is. This gap
  is the hysteresis that keeps the motor from chattering.
- **The 20 ticks of the motor** are a difference of timestamps:
  `now - motor_start >= 20`. A vent that starts opening on tick 5 is
  `OPENING` on ticks 5 to 24, which is 20 ticks, and `OPEN` on tick 25. The
  timer starts on the tick the state changes into `OPENING` or `CLOSING`.
  Staying in `OPENING` because it is still too hot does not restart it.
- **The 120 ticks of the storm** count from the last time the alarm turned
  off. An alarm that comes back on within the 120 ticks starts the count over
  the next time it turns off.
- **A storm is remembered until the vent is locked.** If the alarm turns on
  and then off again while the vent is still closing, `WIND_ALARM_ON` is
  still produced until the state is `STORM_LOCK`. Rule 3 says the alarm
  closes the vent and `STORM_LOCK` lasts until the alarm has been off for
  120 ticks. Without this memory, a short gust during closing would leave the
  vent `CLOSED`, never locked, and a hot greenhouse would open it again at
  once (checked with the same simulation). The 120 ticks still count from the
  tick the alarm really turned off.

**Where the event code lives: `fsm.c`,** with the state machine, the motor
timer, the storm count and the status word. They belong together: all of
them turn the state and the inputs into what the device does next, and it
keeps `main.c` tidy. It also means they can be unit-tested, since a test
program cannot link `main.o` (both have a `main`). `fsm.c` therefore includes
`sensor.h` (for `reading_t`) and `bits.h` (for the status word).

**The timers and the storm memory are a struct** (`fsm_timers_t`) that
`main` owns and passes to the event code by pointer, as in HW3. Each test
makes a fresh one. It holds the motor's start time, the time the alarm last
turned off, the alarm bit from the tick before (to see it turn off), and
whether a storm is waiting to be locked.

**One function runs a tick:** `fsm_tick` updates the alarm memory, gets the
event by priority (`fsm_event`), steps the state machine (`fsm_step`), and
starts the motor timer on the same tick that the state changes into
`OPENING` or `CLOSING`. That is what makes opening from tick 5 end on tick
25: the event code alone would only see the new state on tick 6.

## Transition table

Each cell is `next state, action`. The actions are only markers
for the trace and the history stack, because the LEDs are set from the state.
Cells
marked "—" cannot happen (for example, the motor finishing while `CLOSED`).
They are filled in as "stay, `A_NONE`", so that no cell is left out.

| State \ Event | `COOL_ENABLE`, `HUMID_TRIGGER_HI` | `COOL_DRY`                 | `MOTOR_OPEN_20`  | `MOTOR_CLOSE_20`   | `WIND_ALARM_ON`            | `WIND_ALARM_OFF_120`      |
| ------------- | --------------------------------- | -------------------------- | ---------------- | ------------------ | -------------------------- | ------------------------- |
| `CLOSED`      | `OPENING`, `A_START_OPEN`         | `CLOSED`, `A_NONE`         | —                | —                  | `STORM_LOCK`, `A_STORM`    | —                         |
| `OPENING`     | `OPENING`, `A_NONE`               | `OPENING`, `A_NONE`        | `OPEN`, `A_STOP` | —                  | `CLOSING`, `A_START_CLOSE` | —                         |
| `OPEN`        | `OPEN`, `A_NONE`                  | `CLOSING`, `A_START_CLOSE` | —                | —                  | `CLOSING`, `A_START_CLOSE` | —                         |
| `CLOSING`     | `CLOSING`, `A_NONE`               | `CLOSING`, `A_NONE`        | —                | `CLOSED`, `A_STOP` | `CLOSING`, `A_NONE`        | —                         |
| `STORM_LOCK`  | `STORM_LOCK`, `A_NONE`            | `STORM_LOCK`, `A_NONE`     | —                | —                  | `STORM_LOCK`, `A_NONE`     | `CLOSED`, `A_STORM_CLEAR` |

`COOL_ENABLE` and `HUMID_TRIGGER_HI` are two separate columns in the code
with the same cells, shown here as one column. That gives 5 × 7 = 35 cells.

**Still to come:** the drawing of the state machine.

## Decisions where a rule is silent

1. **Cool and dry while `OPENING`** (the sheet's example: "the temperature
   falling while OPENING"): **finish opening, then close.** The cell
   `OPENING` + `COOL_DRY` stays `OPENING`. Example: the vent starts opening on
   tick 0 and it is cool and dry from tick 5. It is `OPEN` on tick 20; on tick
   21 "cool and dry", produced every tick, starts the closing; it is `CLOSED`
   on tick 41. Each movement takes its full 20 ticks, and nothing extra has to
   be remembered. (Reversing at once was the other choice. Reversing for only
   as long as the vent had moved would break rule 2.)
2. **Too hot or too humid while `CLOSING`: finish closing, then open**, the
   same way. The cells `CLOSING` + `COOL_ENABLE` and `CLOSING` +
   `HUMID_TRIGGER_HI` stay `CLOSING`. Example: closing from tick 0, too hot
   from tick 5: `CLOSED` on tick 20, opening from tick 21, `OPEN` on tick 41.
   A closing because of a storm is never reversed in any case: the wind
   events come before the climate events, and a storm is remembered until
   the vent is locked (see Events).

   Both 1 and 2 rely on the motor events coming first in the priority order.
   If "cool and dry" came before `MOTOR_OPEN_20`, it would win every tick, the
   cell would keep the vent `OPENING`, and it would never finish.
3. **High humidity while the temperature is low** (the sheet's example, for
   instance 15 °C and 90 %RH). Rule 1 answers it. "Open at or above 28.0 °C
   **or** 85 %RH" opens the vent at any temperature. "Close at or below
   24.0 °C **when the humidity is also** at or below 75 %RH": at 15 °C the
   temperature half is met, so the vent closes once the humidity is down to
   75 %RH. Between 75 and 85 %RH nothing changes. A "too cold to open"
   exception would break the "or 85 %RH" half of rule 1.
4. **Wind while `OPENING` or `OPEN`: go to `CLOSING`, then to `STORM_LOCK`
   when the vent is shut.** Rule 3 says the alarm closes the vent, and rule 2
   says closing takes 20 ticks, so going straight to `STORM_LOCK`, as if the
   vent were already shut, would break rule 2. When the 20 ticks are up the
   vent is `CLOSED`, and on the next tick `WIND_ALARM_ON` (the alarm, or the
   remembered storm) locks it.
5. **Wind while `CLOSING`: keep closing, and lock when shut,** the same as 4.
   The cell `CLOSING` + `WIND_ALARM_ON` stays `CLOSING`, and the motor timer
   is not restarted.
6. **Wind going off and on within the 120 ticks** (the sheet's example).
   Rule 3 answers it. `STORM_LOCK` "lasts until the alarm has been off for 120
   ticks", and an alarm that came back on has not been off for 120 ticks, so
   the count starts over from the last time it turned off.
7. **Leaving `STORM_LOCK`** goes to `CLOSED`: the vent is shut and no longer
   locked. If it is hot or humid, the next tick's event opens it.
8. **Wind while `CLOSED`** goes to `STORM_LOCK`, as rule 3 says ("from any
   state"). The vent is already shut, so the motor does not run.

## Status word

One `uint32_t` per record, packed and unpacked with HW1's `get_field` and
`set_field`.

| Bits | Field         | Width |
| ---- | ------------- | ----- |
| 0    | `MOTOR_OPEN`  | 1     |
| 1    | `MOTOR_CLOSE` | 1     |
| 2    | wind alarm    | 1     |
| 3    | unused, 0     | 1     |
| 4–6  | state         | 3     |
| 7–31 | unused, 0     | 25    |

The flags are at the same bit positions as in `LEDS_REG`, so the lowest two
bits of `status` always equal `leds` on the same trace line. It is the order
of HW1's status word: one-bit flags first, the 3-bit field at bit 4. The
state is the second hex digit. Examples:

| Moment                 | Status |
| ---------------------- | ------ |
| `CLOSED`               | `0x00` |
| `OPENING`, motor open  | `0x11` |
| `OPEN`                 | `0x20` |
| `CLOSING`, motor close | `0x32` |
| `STORM_LOCK`, wind on  | `0x44` |

The `OPENING` value matches the sheet's example trace line
(`state=OPENING … leds=0x1 status=0x00000011`).

## The store

- **`record_t`:** the reading, the state at that moment, and the status word.
- **The key** is the temperature in 0.5 °C steps, `floorf(temp_c * 2)`. A
  temperature exactly on a step boundary goes into the step **above**:
  21.0, 21.1, 21.2, 21.3 and 21.4 share key 42, and 21.5 to 21.9 share key 43.
  21.5 rounds up, as in scientific notation. Below 0 °C the same formula
  holds: −0.5 to −0.1 have key −1, and 0.0 to 0.4 key 0. (A plain `(int)`
  cast would give −0.4 to 0.4 all key 0, a step 1.0 °C wide.)
- **`record_compare`** compares two records' keys and returns `<0`, `0` or
  `>0`.
- **Until the tree (week 8)** the store is a sorted array of fixed size, with
  no `malloc`: at most 5000 records, which is 83 minutes of ticks. When it is
  full, inserting returns an error. This is to be revisited once `malloc` is
  covered.
- **Find by key** calls a function once for every record with that key, the
  same way the in-order walk calls one for every record.
- **`store_height`** returns −1 ("not a tree yet") until the tree exists. Its
  tests are written in week 8.
- **Duplicate keys: every record is kept.** A count, as in HW4's tally,
  would keep one record per key and count the rest. Here two records with one
  key are not the same: 30.1 and 30.4 °C share key 60 but differ in
  temperature, tick and state. With a count, the sheet's test "insert
  duplicates and find every one of them" could not pass, and the report could
  not list a reading that was only counted. In the sorted array, a new record
  goes after the records with an equal key, so equal keys stay in tick order.
  Whether the tree (week 8) keeps them as a list in each node or as a
  tie-break on the timestamp is decided then: in the array the two are the
  same.

## Still to come

- the drawing of the state machine
- the RGB color of each state
- how the stack grows, and why (week 7)
- the exact definition of the report (week 9)
- the answer to the 2038 question
- one paragraph that defends or criticizes the given states
- one paragraph that compares the stack with an alternative from the reading,
  with the cost of each operation
