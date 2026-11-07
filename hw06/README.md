# HW6: the device loop

*Drafted by Claude at the user's request, for the user to rewrite. Progress,
step by step, is in [PROGRESS.md](PROGRESS.md).*

## How to build and test

Run these from `hw06/`:

```bash
make                          # builds replay with gcc -std=c11 -Wall -Wextra -g
make test                     # the unit tests, then the regression test
make clean                    # removes everything make built
./replay scenarios/NAME.txt   # replays one scenario and prints its trace
```

- `tests/test_hw06.c` holds the unit tests. It prints PASS or FAIL per test
  and a summary, and exits with 1 if any test fails. It is built with
  `-fsanitize=address,undefined`.
- `regress.sh` runs `./replay` on every scenario in `scenarios/` and compares
  the trace with the file of the same name in `expected/`, as HW2's
  `regress.sh` did.
- `iom361_r4.h` and `iom361_r4.c` are copied unchanged.

## What the tests check

*Written by Claude, for the user to review.*

| Test                   | What it checks                                                                                                                                                          |
| ---------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| sensor                 | 91.0 °C and 40.0 %RH read back exactly, with the timestamp; −12.3 °C rounds correctly; a wrong base returns 1 and changes nothing                                       |
| panel edges            | a press, a release, a held switch, and several switches at once                                                                                                         |
| panel LEDs             | `panel_led` and `panel_toggle` keep the other LEDs (`0x5` → `0x7` → `0x3` → `0x2` → `0xA`); a bad `n` is −1 and changes nothing; after a failed read nothing is written |
| scenario load          | comments and blank lines, a comment after the fields, hex without `0x`                                                                                                  |
| scenario hold          | values hold between lines; nothing is in force before tick 0; the last tick                                                                                             |
| scenario wrong         | the line number for: no start line, a missing field, ticks that do not increase, an extra word, a first tick that is not 0; 1 for an empty file                         |
| clock                  | the start time, 59 seconds later, midnight, 28 February 2027 to 1 March                                                                                                 |
| `scenarios/press.txt`  | the replay: a press, a hold, a release, two presses on one tick, a press that turns an LED off again                                                                    |
| `scenarios/values.txt` | the replay: values holding between lines, 91.0 °C, −12.3 °C, 0 and 100 %RH, and the date changing at midnight                                                           |

The expected traces in `expected/` were made by running `./replay` and then
checking every line by hand against the comments in the scenario.

## Decisions

Who decided each item, and the reason given. "Settled" means the HW6 handout,
a lecture slide, or a project decision carried over determines it.

| Decision                                                   | Choice                                                                                          | Who                                           | Reason given                                                                                                                                                        |
| ---------------------------------------------------------- | ----------------------------------------------------------------------------------------------- | --------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `sensor_read` rounding                                     | integers only, from the raw value                                                               | the user (a project decision)                 | none given                                                                                                                                                          |
| `sensor_read` with a `NULL` `r`                            | not allowed; the contract says so                                                               | the user (a project decision)                 | none given                                                                                                                                                          |
| A conversion helper                                        | no: `sensor_read` only                                                                          | the user (a project decision)                 | "do it like the homework"                                                                                                                                           |
| H1: what `panel_led` and `panel_toggle` return             | 0; −1 for a bad `n`; otherwise the iom361 `rc`, and no write after a failed read                | the user                                      | none given                                                                                                                                                          |
| H2: what `scenario_load` reads from                        | an open `FILE *`; `main` opens the file named on the command line                               | the user, on Claude's suggestion              | the handout's `scenario_load` returns only 0 or a line number, so a file that cannot be opened is reported by `main`                                                |
| H3: how the line in force is handed back                   | copied into a struct the caller passes in                                                       | the user                                      | "to make it clear to the code reviewer what is going on inside the function"                                                                                        |
| H4: `sensor_read` failing mid-run                          | stop: the tick and the `rc` on `stderr`, exit 1                                                 | the user                                      | none given                                                                                                                                                          |
| A line with an extra word                                  | wrong line                                                                                      | settled                                       | week 6 slide 29                                                                                                                                                     |
| A first data line whose tick is not 0                      | wrong line                                                                                      | settled                                       | the handout's `main` replays every tick from 0                                                                                                                      |
| A bad scenario                                             | the line number on `stderr`, exit 1                                                             | settled                                       | week 6 slide 31: "report line, stop"                                                                                                                                |
| A file with no start line at all (empty, or only comments) | `scenario_load` returns 1                                                                       | the user                                      | none given                                                                                                                                                          |
| The clock                                                  | in `scenario.c`                                                                                 | the user (a project decision)                 | none given                                                                                                                                                          |
| `scenario_t` visible or opaque                             | visible in `scenario.h`                                                                         | Claude, at the user's request ("if its okay") | "no `malloc`" means `main` owns the 1000-line `scenario_t` as a plain variable, which needs the type to be visible; opaque would need a hidden pool, as HW4's tally |
| `sensor.h` and `panel.h`                                   | written by the user; Claude added the `NULL` note and the full H1 return values to the comments | the user, Claude                              | the comments describe every decision                                                                                                                                |
| Tests built with the sanitizers                            | yes                                                                                             | Claude                                        | the project requires it, and this code is copied into the project                                                                                                   |
