# HW2: Splitting readings.c into modules

The starter `readings.c` reads sensor readings (tick, temperature, humidity)
from standard input and prints a summary: counts, min/max/mean, the longest run
above a temperature threshold, and a histogram. HW2 splits it into modules
without changing a single character of its output.

## How to build and test

```bash
make            # builds ./readings
make test       # runs the unit tests, then the regression test (regress.sh)
make clean      # removes everything the build produced
```

Run the program:

```bash
./readings < data/normal.txt        # default threshold 30.0 C
./readings 25 < data/edge.txt       # threshold 25.0 C
```

## Module diagram

An arrow means "calls a function in".

```
            main
          /   |   \
         v    v    v
   reading  stats  histogram
```

`main` calls all three modules. The modules do not call each other.

## How the program works

### Build (Makefile)

*Written by Claude at the user's request.*

- One `.o` per `.c`, compiled with `gcc -std=c11 -Wall -Wextra`.
- Each `.o` depends only on its own `.c` and the headers that `.c` includes.
  Changing `histogram.c` rebuilds only `histogram.o` and the program. Changing
  `stats.h` rebuilds `main.o`, `stats.o`, and the unit test.
- `make test` runs the unit tests first, then `sh regress.sh`. It stops at the
  first failure.

### Unit tests (tests/test_stats.c)

*Harness written by Claude. Test cases to be written by the user.*

- A `check()` helper prints PASS or FAIL for each test and counts the results.
  The program prints a summary and exits with 1 if any test failed.
- The four required tests (one value, max at start and end, value equal to the
  threshold ends a run, recursive max on 10,000 values) all pass, 8 checks in
  total. The user decides what each test checks, and Claude writes the code.
  Each test was also run against a deliberately broken copy of `stats.c` to
  confirm it can fail.
- **One value** (test case by the user): `{23.5}` with `n = 1`. Max, min,
  and mean must all be 23.5. This is the base case of the recursion and the
  smallest `n` allowed. It is also the only test of `stats_min` and
  `stats_mean`.
- **Max at the ends** (test cases by the user and Claude): `{101, 40, 38, 66}`
  and `{40, 38, 66, 101}`, both expecting 101. Same numbers, only the position
  of the max changes. Each half catches a different bug: a version of
  `stats_max` that ignores the first element fails the first half, and one
  whose base case stops one element early fails the second.
- **Value equal to the threshold ends a run** (the user's idea, array by
  Claude): `{31, 32, 33, 30, 34, 35, 20}` with threshold 30.0. Expected:
  length 3, start 0. The 30.0 is not above 30.0, so it breaks the first run.
  The user first thought the value at the threshold was the last one in the
  run. Working through it showed it is the largest value that does not count.
  With `>=` instead of `>`, indexes 0-5 would become one run of 6, and the
  length check fails.
- **Recursive max on 10,000 values** (fill with a counter: the user's idea;
  counting direction: Claude's suggestion): a loop fills the array from a
  counter that goes up by one each time, so `a[0] = 1` and `a[9999] = 10000`.
  Expected max: 10000. The user first had the counter count down from 10000,
  which put the max at index 0. Claude pointed out that the max would then be
  found by the very first call, so a `stats_max` that stopped partway through
  would still pass. The user agreed that the test has to check the maximum,
  not only that the recursion survives, and switched to counting up. A copy of
  `stats_max` that looks at only the first 5,000 values fails the counting-up
  test but passes the counting-down one.

### main (main.c)

*Moved from the starter by Claude at the user's request.*

- Declares the arrays `ticks`, `temps`, `hums` (size `MAX_READINGS`) and the
  threshold (default 30.0).
- Checks the command line: more than one argument prints a usage message, and
  a threshold that is not a number prints an error. Both return 1.
- Calls `reading_read`, prints the readings and skipped counts, and stops with
  "no readings, no summary" if there are none.
- *Written by the user:* prints the temperature and humidity lines by calling
  `stats_min`, `stats_max`, and `stats_mean` once with `temps` and once with
  `hums`. The format strings are copied unchanged from the starter so the
  output matches byte for byte.
- *Written by the user:* calls `stats_longest_run` and prints either "never"
  or the run, using its own `ticks` array to turn the start index into tick
  numbers.
- *Written by the user:* calls `histogram_print(temps, r.count)` last, so the
  output order matches the starter: counts, temperature, humidity, longest
  run, histogram.

### Reading (reading.h, reading.c)

*Moved from the starter by Claude at the user's request. Struct design by the
user.*

- `reading_read` reads lines from standard input with `fgets` and fills the
  three arrays through its parameters. It returns `struct reading_result`
  with `count` and `skipped`.
- A `static` helper, `is_blank_or_comment`, skips leading spaces and tabs and
  reports whether the line is blank or starts with `#`. Those lines are
  ignored and not counted.
- A line where `sscanf` does not read three numbers is counted as skipped.
- After `max` readings it prints a warning to stderr and stops.

### How a line of input reaches the arrays

*Explained by Claude at the user's request.*

Each input line is one reading with three columns: tick (time in seconds,
`int`), temperature (°C, `float`), and humidity (%RH, `float`). The tick is a
timestamp, not a sensor value. It is only used to print where the longest run
starts and ends. For the line `60  25.0  68.8`:

1. `main` declares three empty arrays, one per column: `int ticks[]`,
   `float temps[]`, `float hums[]`, each `MAX_READINGS` long.
2. `main` calls `reading_read(ticks, temps, hums, MAX_READINGS)`. Arguments
   match parameters by position, not by name. Passing an array passes where it
   lives, so `reading_read` writes straight into `main`'s arrays.
3. `fgets` copies one line of the file into `line` as text. Blank lines and
   comments are dropped here.
4. `sscanf(line, "%d %f %f", &ticks[r.count], &temps[r.count], &hums[r.count])`
   splits the text: `60` goes to `ticks`, `25.0` to `temps`, `68.8` to `hums`.
   The order in the format string matches the column order. All three use the
   same index, so one reading stays lined up across the three arrays.
5. If `sscanf` converted all 3 values, `r.count++` moves to the next slot.
   Otherwise `r.skipped++`, and the next good line overwrites the partial data.

When the loop ends, `r.count` is how many slots are filled. That number is the
`n` passed to the stats functions, for example `stats_min(temps, r.count)`.

### Stats (stats.h, stats.c)

*Written by the user.*

- `stats_max(a, n)`: recursive maximum, described under the user's decisions.
- `stats_min(a, n)`: starts at `a[0]` and keeps the smallest value.
- `stats_mean(a, n)`: adds the values in a `float` in index order, then returns
  `sum / n`, the same arithmetic as the starter.
- `stats_longest_run(a, n, threshold)`: finds the longest run of values
  strictly above the threshold (a value equal to it ends a run). It returns
  `struct stats_longest_run_result` with `length` and `start` (the index of
  the first reading in the run, -1 if there is none). It does not print.
  `main` does that, because only `main` has the `ticks` array.
- All of them assume `n >= 1`. `main` makes sure of this by stopping when
  there are no readings.

### Histogram (histogram.h, histogram.c)

*Written by the user.*

- `histogram_print(a, n)` returns `void`: its job is printing, so `main` needs
  nothing back.
- It counts the values into 20 bins of 5 °C (0 to 100). Values below 0 go in
  the first bin and values of 100 or more go in the last. Then it prints each
  non-empty bin as a row of `*` with the count.
- `NUM_BINS` and `BIN_WIDTH` are private `#define`s in `histogram.c`, because
  no other file uses them.

### Regression test (regress.sh)

*From the starter.*

- Runs `./readings` on every file in `data/` (and `edge.txt` with threshold 25)
  and compares the output with `expected/` using `diff`.

## The user's decisions

- **Division of work.** The user writes the parts the course is about: the
  module interfaces (headers), the recursive `stats_max`, removing the globals,
  deciding what is `static`, and the test cases. Claude writes the Makefile
  (outside the scope of the class, per the professor), the test harness
  boilerplate, and the documentation.
- **Order of work.** The user chose to have the Makefile and test scaffold done
  first, so that the regression test can run after every refactoring step.
  Next come the headers, starting with `stats.h`.
- **Using expected/.** The starter already matches `expected/` exactly, so the
  user treats it as the behavior that must not break, and traced
  `data/edge.txt` against `expected/edge.txt` to find the rules each module has
  to keep.
- **Module dependencies.** The user first thought the modules would depend on
  each other in a chain (stats, then reading, then histogram). Claude pointed
  out that this was the order the steps run in, not who calls whom. The user
  then settled on `main` as the only caller: it gets the arrays from `reading`
  and passes them to `stats` and `histogram`. A module that needs the data does
  not need the module that produced it, so `stats.c` never includes
  `reading.h` and can be tested on any array.
- **Data lives in main.** The arrays `ticks`, `temps`, `hums` and the counts
  are declared in `main` and passed as parameters. They stay three parallel
  arrays (one reading is `ticks[i]`, `temps[i]`, `hums[i]`), so a plain float
  array can be handed to `stats_max`. `MAX_READINGS` goes in `reading.h`,
  because both `main` and `reading` need it. The other defines stay private in
  their own `.c`.
- **Keeping `<string.h>`.** The starter includes it but never uses it. The
  user chose to keep it in `main.c` for later homework and the project.
- **Returning more than one result.** `reading_read` must give back both
  `count` and `skipped`. The user chose to return a small struct holding the
  two counts. The arrays are filled through the parameters, so they are not in
  the struct.
- **One function per statistic.** The user first declared separate functions
  for temperature and humidity (`stats_temp_min`, `stats_hum_min`, ...).
  Claude pointed out that rule 2 asks for one function per statistic, called
  twice, and that the bodies would be identical. The user collapsed them into
  `stats_min`, `stats_max`, and `stats_mean`.
- **Longest run returns a struct.** Like `reading_read`, the longest run
  produces two results (length and start). The user defined
  `struct stats_longest_run_result` in `stats.h`, named after the function,
  following the `reading_result` pattern. The field names `length` and `start`
  describe what the caller gets, rather than the loop's `best` and
  `best_start`.
- **Call order is output order.** The user first put the `histogram_print`
  call before the longest-run code. Because each call prints as it runs, that
  would have printed the histogram before the "above" line. The user moved it
  to the end of `main`.
- **Counters in the test file.** `test_stats.c` keeps its `passed` and
  `failed` counts in two file-scope `static` variables. The user decided this
  is fine: the no-globals rule is about the readings program, which has none,
  and `static` keeps the counters private to the test file.
- **Recursive maximum.** The user wrote `stats_max(a, n)` to replace the
  starter's `max_temp(i)`, which only worked on the global array. Base case:
  `n == 1` returns the only element. Recursive step: compare the first
  element with the max of the rest (`a + 1`, `n - 1`). It assumes `n >= 1`.
