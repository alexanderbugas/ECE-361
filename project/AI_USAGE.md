# AI usage: project

*Written by Claude.*

## Tools

- **Claude (Claude Code, in VS Code).** Used throughout, at the user's
  request.

## What it was used for

- **Planning.** Claude read the project sheet, the week 6 lecture, the HW6
  handout and the user's HW1 to HW5 code, and wrote `MODULES.md`: a
  description of each module, the homework code it can be built from, and a
  guide to every decision the project needs. The user made the decisions;
  Claude then moved them into `SPEC.md` and `README.md`, and wrote
  `PROGRESS.md` to track the work.
- **Checking facts before writing them down.** Claude compiled and ran small
  programs to check the rounding methods against iom361, the store's key
  formula for every tenth from −50.0 to 150.0 °C, the status-word values,
  and the storm-closing behavior under two priority orders.
- **Steps 1 to 4** (at the user's request): the test setup and `make test`,
  the states in `fsm.h`, `bits.[ch]` copied from HW1, and the status word's
  pack and unpack in `fsm.c`, with their tests.
- **Steps 5 to 7** (at the user's request): `sensor.h` (the HW6 interface),
  `store.h`, and the sorted-array `store.c` with the sheet's store tests.
- **Steps 8 and 9** (at the user's request): the events, actions and
  transition table, `fsm_step`, and the event code with its timers
  (`fsm_event`, `fsm_tick`), with tests for every cell, every threshold and
  every silent rule. To check that the tests can fail, Claude broke two
  things in a scratch copy (the wind before the motor in the priority, and
  no storm memory) and confirmed the storm tests caught both.
- **Steps 10 to 13** (at the user's request): copied `sensor.c` and
  `scenario.[ch]` unchanged from `hw06/`, wrote the project's `main.c` from
  HW6's loop with the state machine in place of the toggling, added
  `fsm_leds` and the color table to `fsm.c`, and the sensor, scenario and
  clock tests. Ran `./vent` on the sheet's example scenario and on a full
  open-and-close cycle to check the trace.
- **Step 14** (at the user's request): wrote the sheet's scenarios 1, 3, 4, 5
  and 7. Before saving the expected traces, Claude worked out the state at
  70 key ticks by hand from the rules and checked `./vent`'s output against
  them (no mismatches). Wrote `regress.sh` so that `make test` ends with one
  passed/failed line for the unit tests and scenarios together, and checked
  that a wrong expected trace makes `make test` fail.

## What it got wrong

- In the decision guide, Claude first presented questions the project sheet
  already answers (for example, whether events fire every tick or only on a
  change) as if they were the user's choice. The user pointed this out, and
  Claude then marked each decision as settled by a rule or as the user's.
- Claude wrote that two separate opening events would let the trace show why
  the vent opened. The sheet's trace line has no event in it, so this was
  wrong, and it was corrected.
