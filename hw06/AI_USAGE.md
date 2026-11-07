# AI usage: HW6

*Written by Claude.*

## Tools

- **Claude (Claude Code, in VS Code).** Used throughout, at the user's
  request.

## What it was used for

- **Planning.** Claude read the HW6 handout and the week 6 lecture and wrote
  `PROGRESS.md`: the steps, what the handout settles, and the decisions left
  to the user (H1 to H4), each with its options. The user made the decisions.
- **Setup** (at the user's request): copied `iom361_r4.[ch]` unchanged from
  `project/`, and wrote the `Makefile`, `regress.sh`, the empty
  `tests/test_hw06.c`, `README.md` and this file.
- **Parts 1 and 2** (at the user's request): the user wrote `sensor.h` and
  `panel.h`; Claude pointed out the missing includes and semicolon (which the
  user fixed), added two notes to the comments, and wrote `sensor.c`,
  `panel.c` and their tests. To check that the tests can fail, Claude broke
  `panel_led` (writing `1u << n` without reading first) and the rounding
  (without the added half) in a scratch copy, and the tests caught both.
- **Part 3, the scenario** (at the user's request): the user wrote
  `scenario.h` and a first `scenario.c`, from pseudocode Claude gave. Claude
  reviewed it, then at the user's request fixed `scenario_load` (the loop,
  the line buffer, the start line, the checks and the store),
  `scenario_at` (it copied line 1 instead of the line found) and
  `scenario_last_tick` (`sc-count`, and the logic reversed), and wrote the
  scenario and clock tests.
- **Part 3, `main.c`:** Claude wrote the pseudocode into `main.c`; the user
  wrote `main()` from it. At the user's request Claude added the side
  comments, labeled the pseudocode, and fixed five things that stopped it
  compiling: the include descriptions without comment markers, `int` for the
  `FILE *` from `fopen`, the `unit32_t` typo, the undeclared `rc`, and `int`
  where the registers are `uint32_t`.
- **Part 4, the regression test:** Claude wrote `scenarios/press.txt` and
  `scenarios/values.txt`, ran `./replay` on them, checked every line of the
  traces against the scenarios' comments, and saved them in `expected/`.
  Changing one value in an expected trace made `regress.sh` fail, so the
  comparison really runs.

## What it got wrong

- In `PROGRESS.md`, Claude first wrote that writing a failed read's value
  (`0xDEADBEEF`) back to the LEDs "would light most of the LEDs". Its low four
  bits are `0xF`, so it lights three or all four, depending on the change;
  this was corrected.
