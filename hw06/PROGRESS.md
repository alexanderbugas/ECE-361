# HW6 progress

*Kept by Claude, from the HW6 handout (Revision 0, Nov 3). One row per step. A
step is marked done only when `make test` passes with no warnings. Due Sun Nov
8, 11:59 pm, pushed to `hw06/`. Shine: opens Mon Nov 9, due Thu Nov 12,
without supporting materials, so every line has to be one the user can
explain.*

*Order of the work: project steps 1 to 9 first (see
[project/PROGRESS.md](../project/PROGRESS.md)), then these steps, then
`sensor.c`, the scenario code with the clock, and the loop are copied into
`project/`.*

## Status

| #   | Step                                         | Handout part | Status   |
| --- | -------------------------------------------- | ------------ | -------- |
| 1   | `iom361_r4.[ch]`, `Makefile`, `tests/` setup | 4            | not done |
| 2   | `sensor.h`, `sensor.c`                       | 1            | not done |
| 3   | `panel.h`, `panel.c`                         | 2            | not done |
| 4   | `scenario.h`, `scenario.c`, with the clock   | 3            | not done |
| 5   | `main.c`: `./replay scenario.txt`            | 3            | not done |
| 6   | `scenarios/`, `expected/`, `regress.sh`      | 4            | not done |
| 7   | `README.md`, `AI_USAGE.md`                   | deliverables | not done |

## The steps

Each step: Claude writes the code and its tests, runs `make test` and shows
the result, explains every change, and updates this file and the README.
Nothing is committed. The next step starts only when the user says so.

### 1. Setup

- **Settled by the handout:** `iom361_r4.h` and `iom361_r4.c`, unchanged
  (copied from `project/`). `make` builds `replay` from all modules and
  `iom361_r4.c` with no warnings; each `.o` lists the headers it includes;
  `make test`; `make clean`.
- **Tests:** `tests/test_hw06.c`, PASS or FAIL per test, a nonzero exit
  status on failure, with HW5's `check` helper
  ([hw05/tests/test_lock.c:32-40](../hw05/tests/test_lock.c#L32-L40)).
- **Carried over from the project:** the tests are built with
  `-fsanitize=address,undefined`, so the code copied into the project already
  passes the sheet's sanitizer requirement.

### 2. `sensor.h` and `sensor.c` (Part 1)

- **Settled by the handout:**
  `typedef struct { time_t t; float temp_c; float rh_pct; } reading_t;` and
  `int sensor_read(uint32_t *base, time_t now, reading_t *r)`. It reads
  `TEMP_REG` and `HUMID_REG`, keeps bits 19..0, converts, rounds to 0.1, and
  stores the result with the timestamp `now` in `*r`. It returns 0, or the
  iom361 return code of a failed read, and then it changes nothing. No
  `printf`, and no calls to `_iom361_set_…`.
- **Carried over from the project's decisions:** rounding with integers only
  (`(st * 2000u + (1u << 19)) >> 20` minus 500 tenths, and
  `(srh * 1000u + (1u << 19)) >> 20`, from week 6 slides 8 to 10); `r` must
  not be `NULL` (said in the contract); no conversion helper, `sensor_read`
  only.
- **Tests (from the handout):** a reading of 91.0 °C reads back as 91.0; a
  failed read changes nothing.

### 3. `panel.h` and `panel.c` (Part 2)

- **Settled by the handout:**
  - `uint32_t panel_pressed(uint32_t now, uint32_t before)`: the switches that
    went from off to on, one bit per switch (`now & ~before`, slide 22).
  - `uint32_t panel_released(uint32_t now, uint32_t before)`: the switches
    that went from on to off.
  - `int panel_led(uint32_t *base, int n, int on)`: turns LED `n` on or off by
    read-modify-write, so that every other LED keeps its state; −1 if `n` is
    not 0 to 31 (slides 19 and 20).
  - `int panel_toggle(uint32_t *base, int n)`: toggles LED `n`, the same way.
- **Tests (from the handout):** pressed and released for a press, a release
  and a held switch; `panel_led` and `panel_toggle` keep the other LEDs.
- **The handout's example Shine question:** "Why does your `panel_led` read
  the register before it writes it, and what goes wrong if it doesn't?"
- **Yours, before this step:** what `panel_led` and `panel_toggle` return on
  success, and when an iom361 read or write fails (0 and the `rc`, as
  `sensor_read` does, or something else). The handout fixes only −1 for a bad
  `n`.

### 4. `scenario.h` and `scenario.c` (Part 3), with the clock

- **Settled by the handout:** the project sheet's format (`#` comments, a
  `start <timestamp>` line, then `tick temp rh switches`, the switches in hex
  and the ticks increasing), loaded into a `scenario_t` that holds at most
  1000 lines, **no `malloc`**. `scenario_load` returns 0, or the number of
  the first wrong line. A function gives the line in force at a tick: the
  values hold between lines.
- **From the week 6 lecture (slide 29):** drop the comment first by putting a
  `'\0'` at the `#` (`strchr`), then
  `sscanf(line, "%d %f %f %x %1s", …)`: 4 means good, fewer means a field is
  missing, 5 means something extra.
- **Carried over from the project's decisions:** the clock lives here too:
  `now = start + tick`, printed in UTC with `gmtime` and `strftime` as
  `2026-10-06 08:00:46`.
- **Tests (from the handout):** a load with comments and blank lines; values
  holding between lines; a scenario without a start line, with a missing
  field, and with ticks that do not increase. **Added for the project:** the
  sheet's clock tests (the start time prints as given, 59 seconds later,
  one second after 23:59:59 is the next day's 00:00:00, and 28 February 2027
  to 1 March).
- **Yours, before this step:**
  - what `scenario_load` takes: a file name, or an open `FILE *`. A
    `FILE *` lets a test load from a temporary file or a string written to
    one; a file name keeps `main` shorter;
  - the shape of the "line in force" function: return a pointer to the line,
    or copy it into a struct the caller passes in;
  - what the loader does with a line that has more than the four fields
    (`%1s` reads 5): count it as wrong, or ignore the extra word;
  - what happens when the first data line's tick is not 0. There is no line
    in force yet on ticks before it: either a wrong file, or nothing to
    replay until then.

### 5. `main.c` (Part 3)

- **Settled by the handout:** `./replay scenario.txt`. It initializes iom361
  with 4 switches and 4 LEDs and turns the display off. For every tick from 0
  to the last one, the line in force sets the "world" with
  `_iom361_set_sensor1` and `_iom361_set_switches`; the clock is
  `start + tick`; it reads the sensor and the switches **through the
  registers**, toggles LED k for every switch k just pressed, and prints one
  line:
  `2026-10-06 08:00:02 temp=21.5 rh=40.0 switches=0x1 pressed=0x1 leds=0x1`.
- **Yours, before this step:** what `main` prints and returns for a bad
  scenario (for example the line number on `stderr`, exit status 1), and for
  a failed `sensor_read`.

### 6. Regression tests (Part 4)

- **Settled by the handout:** at least two scenarios in `scenarios/` with
  their expected traces in `expected/`. `make test` runs the unit tests, then
  compares `./replay` on each scenario with its expected trace, with a
  `regress.sh` that uses `diff`, as in HW2
  ([hw02/regress.sh](../hw02/regress.sh)).

### 7. `README.md` and `AI_USAGE.md`

- `README.md`: how to build and test (the handout), plus, as in every
  homework, a record of the user's decisions and who wrote what.
- `AI_USAGE.md`: which tools were used, for what, and one thing a tool got
  wrong.

## Copied into the project afterwards

| From `hw06/`                  | Into `project/`              | Changes for the project                                                              |
| ----------------------------- | ---------------------------- | ------------------------------------------------------------------------------------ |
| `sensor.[ch]`                 | `sensor.[ch]`                | none                                                                                 |
| `scenario.[ch]` and the clock | `scenario.[ch]`              | none                                                                                 |
| the loop in `main.c`          | `main.c`                     | 1 switch and 2 LEDs; the state machine instead of toggling; the project's trace line |
| `regress.sh`                  | `regress.sh`                 | the project's scenarios                                                              |
| `panel.[ch]`                  | only if the project needs it | the project sets its LEDs from the state                                             |
