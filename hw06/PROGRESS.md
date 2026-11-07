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

| #   | Step                                         | Handout part | Status       |
| --- | -------------------------------------------- | ------------ | ------------ |
| 1   | `iom361_r4.[ch]`, `Makefile`, `tests/` setup | 4            | **complete** |
| 2   | `sensor.h`, `sensor.c`                       | 1            | **complete** |
| 3   | `panel.h`, `panel.c`                         | 2            | **complete** |
| 4   | `scenario.h`, `scenario.c`, with the clock   | 3            | **complete** |
| 5   | `main.c`: `./replay scenario.txt`            | 3            | **complete** |
| 6   | `scenarios/`, `expected/`, `regress.sh`      | 4            | **complete** |
| 7   | `README.md`, `AI_USAGE.md`                   | deliverables | **complete** |

## The steps

Each step: Claude writes the code and its tests, runs `make test` and shows
the result, explains every change, and updates this file and the README.
Nothing is committed. The steps are done in batches (1–3, 4–5, 6–7), with
a review after each batch.

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
- **Decision H1: what `panel_led` and `panel_toggle` return. Decided by the
  user: option A.** The
  handout fixes only −1 for an `n` that is not 0 to 31. Both functions do a
  read and a write through iom361, and either can fail and set an `rc` (1 bad
  base, 2 offset out of range, 3 not a multiple of 4, 5 not initialized).
  - **Option A: the `sensor_read` pattern.** 0 on success; −1 for a bad `n`;
    otherwise the `rc` of the read or write that failed. If the read fails,
    nothing is written. Example: `panel_led(base, 2, 1)` before
    `iom361_initialize` returns 5. The caller can say why it failed, and the
    three functions that touch registers behave alike.
  - **Option B: success or failure only.** 0 on success, −1 for a bad `n` or
    any iom361 failure. Simpler to check, but a caller cannot tell a bad `n`
    from a broken register.
  - **Either way:** a failed read must not be followed by a write. A failed
    `iom361_readReg` returns `0xDEADBEEF`, whose low four bits are `0xF`, so
    writing it back with one bit changed would light three or all four LEDs.


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
- **Settled: a line with an extra word is wrong.** Slide 29 reads one word
  more with `%1s`, so 5 fields means "something extra", and says "report the
  line number of the first bad line". A `#` comment after the four fields is
  not extra, because it is cut off first.
- **Decided by the user: a file with no start line at all** (empty, or only
  comments) makes `scenario_load` return 1.
- **Settled: a first data line whose tick is not 0 is wrong.** The handout's
  `main` replays "every tick from 0 to the last one", and every tick needs a
  line in force. Before the first line there is none.
- **Decision H2: what `scenario_load` reads from. Decided by the user:
  option B** (an open `FILE *`), on Claude's suggestion. Either way the name
  comes from the command line (`./replay scenario.txt`, `argv[1]`); `main`
  calls `fopen` and reports a file it cannot open. The reason: the handout's
  `scenario_load` returns only 0 or a line number, and with option A a file
  that cannot be opened has neither.
  - **Option A: a file name,** `int scenario_load(scenario_t *sc, const char
    *path)`. `scenario_load` opens and closes the file itself, and `main` is
    one call. A test needs a real file: either fixed test files in a folder,
    or a temporary file the test writes first.
  - **Option B: an open stream,** `int scenario_load(scenario_t *sc, FILE
    *in)`. `main` does the `fopen` and `fclose`, and can report "cannot open
    the file" separately from "line 7 is wrong". A test can write a few lines
    to a `tmpfile()` and load it, with no files to keep in the folder. This
    is the way HW4's `tally_print` took its output stream.
- **Decision H3: how the line in force is handed back. Decided by the user:
  option B,** "to make it clear to the code reviewer what is going on inside
  the function". A line is a
  tick, a temperature, a humidity and the switches.
  - **Option A: a pointer into the scenario,**
    `const scenario_line_t *scenario_at(const scenario_t *sc, int tick)`.
    No copy, and `NULL` can mean "no line in force". The pointer is only good
    while the `scenario_t` exists. This is how HW3's `find_above` handed back
    a pointer into the array ([hw03/ptr.h:40](../hw03/ptr.h#L40)).
  - **Option B: a copy into the caller's struct,**
    `int scenario_at(const scenario_t *sc, int tick, scenario_line_t *out)`,
    returning 0, or −1 if no line is in force. The same output-pointer pattern
    as `sensor_read` and HW3's `minmax`. The caller owns its copy.

### 5. `main.c` (Part 3)

- **Settled by the handout:** `./replay scenario.txt`. It initializes iom361
  with 4 switches and 4 LEDs and turns the display off. For every tick from 0
  to the last one, the line in force sets the "world" with
  `_iom361_set_sensor1` and `_iom361_set_switches`; the clock is
  `start + tick`; it reads the sensor and the switches **through the
  registers**, toggles LED k for every switch k just pressed, and prints one
  line:
  `2026-10-06 08:00:02 temp=21.5 rh=40.0 switches=0x1 pressed=0x1 leds=0x1`.
- **Settled: a bad scenario stops the run.** Slide 31: "bad count: report
  line, stop". `main` prints the line number on `stderr` and exits with 1,
  the way the project's `main.c` already handles a failed
  `iom361_initialize`. A file that cannot be opened is handled the same way.
- **Decision H4: what `main` does when `sensor_read` fails mid-run. Decided
  by the user: option A.**
  With a correct base this cannot happen in iom361, but the check costs one
  `if`, and the sheet says to check every `rc`.
  - **Option A: stop.** Print the tick and the `rc` on `stderr`, exit with 1.
    No trace line for that tick. The trace then ends early, so a regression
    test shows the failure as a difference.
  - **Option B: skip the tick.** Print a warning on `stderr`, print no trace
    line, and go on with the next tick. The run finishes, but the trace has a
    gap, and `sensor_read` left the reading as it was the tick before.

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
