# AI usage, HW2

## Tools

- Claude Code (model: Claude Opus 5.5), in VS Code.

## What Claude was used for

- **Makefile.** Claude wrote `hw02/Makefile`. Makefiles are outside the scope
  of the class, so the user chose not to learn this part. Claude checked it on
  a scratch copy with placeholder modules: no warnings, only the right files
  were rebuilt after touching `histogram.c` and `stats.h`, and `make clean`
  left only source files.
- **Test harness.** Claude wrote the PASS/FAIL harness in
  `tests/test_stats.c` and four stub tests that fail until the user writes
  them. The test cases themselves are the user's.
- **.gitignore.** Claude added `hw02/readings` and `hw02/tests/test_stats` so
  the build output is not committed.
- **Shell commands.** Claude explained `unzip -d` to unpack the starter,
  `git add` for one folder, `touch` to create files, and the `-` in
  `diff - expected/normal.txt` (the user left it out and got "missing
  operand").
- **Where the includes go.** Claude explained which `.c` files need
  `<stdio.h>` and `<stdlib.h>` (based on which functions each module calls),
  that `<string.h>` is unused in the starter and can be dropped, and why
  includes belong in the `.c` files rather than the headers unless the header
  itself needs them.
- **Design questions.** Claude explained what line 74 of `readings.c` does
  (`skipped++` when `sscanf` reads fewer than 3 numbers), that the arrays are
  data owned by `main` rather than by any module, and where each `#define`
  belongs (`MAX_READINGS` in `reading.h`, the rest private, none in stats).
- **Code review.** Claude compiled `stats.c` and `reading.c` on their own and
  explained the errors in `reading.h`: the `typedef` name and the
  `struct reading_result` name did not match, the struct members were arrays
  instead of single ints, and the include guard was missing.
- **Header comments and the struct fix.** At the user's request, Claude
  wrote the contract comments for `stats_max` (in `stats.h`) and
  `reading_read` (in `reading.h`), updated the comment above `stats_max` in
  `stats.c`, added the include guard to `reading.h`, and fixed the struct by
  dropping the `typedef` and naming it `struct reading_result`, which is the
  name the declaration and definition already used.
- **Splitting the input and command-line code.** At the user's request,
  Claude moved lines 48-82 of the starter: the command-line check and
  threshold parsing into `main.c`, and the read loop into `reading_read` in
  `reading.c`, with the blank/comment check pulled out as a `static` helper.
  Claude checked that the first two output lines (readings, skipped) match
  `expected/` for `normal.txt` and `edge.txt`, and that `empty.txt` passes the
  full regression test.
- **Concepts.** Claude explained the parts of a header contract comment
  (takes, returns, assumes) and traced what `stats_min`, `stats_mean` and
  `stats_max` do with `n == 0`. It explained how types are checked: `sscanf`
  checks the input text, the compiler checks calls against the header, and
  only the caller and tests can check that the right array was passed. It also
  traced how one input line ends up in the three arrays, which is now in the
  README.
- **Checking the rules.** After the refactor, Claude ran `regress.sh` (all 4
  pass) and used `nm` on the object files to check that there are no global
  variables and that only the intended functions are visible outside their
  file (the `static` helper `is_blank_or_comment` is not).
- **Unit tests.** The user decides what each test checks, and Claude writes
  the code. For "max at the ends", the user chose `{101, 40, 38, 66}` and
  Claude made the mirror case `{40, 38, 66, 101}`. To check that the test can
  actually fail, Claude ran it in a scratch folder against two deliberately
  broken copies of `stats_max`. Each copy failed exactly one half of the test.
  At the user's request, Claude added min and mean checks to the "one value"
  test and wrote comments in `test_stats.c` explaining why each edge case is
  tested and why `==` on floats is safe there.
  For the threshold test, Claude explained the difference between a
  comparison boundary (`>` vs `>=`) and an array bound, wrote the check, and
  confirmed it fails against a copy of `stats_longest_run` that uses `>=`.
  For the 10,000-value test, the user designed the fill (a counter) and
  Claude wrote the loop and the check. Claude suggested counting up so the
  max is the last value, and showed that a `stats_max` stopping at 5,000
  values fails that version but passes the counting-down one.
- **Comments.** At the user's request, Claude rewrote the comments in all of
  the hw02 source files for clarity. Every public function in `stats.h`,
  `histogram.h`, and `reading.h` now has a contract comment saying what it
  takes, what it returns, and what it assumes (rule 3). Claude also added
  short comments to `main.c`, removed a duplicate copy of the `reading_read`
  contract from `reading.c` and `reading.h`, and fixed the `stats_min`
  comment in `stats.c`, which described recursion although the function uses
  a loop.
- **Project setup (Part 4).** Claude found the iom361 archive and gave the
  `tar` command to unpack it, and moved the five stub files into `project/`
  after the user created them one folder too high. At the user's request,
  Claude added the include guards and `#include` lines to the stubs, drafted
  a comment for each module from `ASSIGNMENT.md` (the user may edit them),
  wrote `project/main.c` (initialize iom361 with 1 switch and 2 LEDs, print
  "Greenhouse vent", return 1 on failure) for the user to review, and wrote
  `project/Makefile`.
- **First SPEC.md.** The user named the seven events and said what each
  means. Claude wrote them into `project/SPEC.md` with the given states.
  Claude pointed out that the user's `WIND_ALARM_ON_120` ("on for 120
  ticks") did not match rule 3, which ends STORM_LOCK when the alarm has been
  off for 120 ticks, and that without an "off" event nothing could end the
  lock. At the user's request, Claude renamed it `WIND_ALARM_OFF_120`. Claude
  also noted that the wind alarm is an on/off switch with no range, and that
  the sensor limits the temperature and humidity it can report.
- **Planning.** Claude summarized the homework into a list of objectives and
  sorted the work into what the user should do to learn it and what is
  tedious enough to hand off.
- **Documentation.** Claude writes this file and keeps `README.md` up to date
  from the user's prompts, marking which ideas were the user's and which were
  Claude's.

## Done by the user, not by AI

- The recursive `stats_max` in `stats.c`. Claude reviewed it and suggested
  removing a leftover global `count` and renaming the parameter, but did not
  write the function.
- The choice of a struct to return `count` and `skipped` from `reading_read`.
  Claude laid out the options (pointer, struct, two functions). The user picked
  the struct.
- `stats_min`, `stats_mean`, and `stats_longest_run` in `stats.c`, the
  `struct stats_longest_run_result` in `stats.h`, and the calls and
  `printf`s for the temperature, humidity, and longest-run lines in `main.c`.
  Claude reviewed each step, compiled it, and explained the compiler errors
  (scope of `best_start`, the struct in the parameter instead of the return
  type, the order of declarations in the header, a redeclared variable).
- The histogram module (`histogram.h`, `histogram.c`) and its call in
  `main.c`. Claude pointed out the `return 0;` in a `void` function and that
  the call had to come after the longest-run line to keep the output order.
- More to be filled in as the user writes the test cases.

## Something the model got wrong

- **Line numbers.** Claude quoted line numbers from the original starter after
  the user had added comments to `readings.c`, so they were off by 7. The user
  noticed and asked Claude to re-read the file.
- **Unclear wording.** Reviewing `stats_longest_run`, Claude said the
  initializer `= {0, -1}` on `result` "could be dropped". It meant only the
  initializer, but the user removed the whole declaration, and the build
  failed with "'result' undeclared". The compiler error showed the problem,
  and Claude explained that the variable still had to be declared.
- **Writing a reason the user never gave.** After writing test 4, Claude
  recorded in the README that the user kept the counting-down fill "because
  the test is about the recursion surviving 10,000 frames". The user had not
  given that reason. The user asked what Claude had done, Claude listed its
  changes and flagged the invented reason, and the README entry was
  rewritten to describe only what happened.
