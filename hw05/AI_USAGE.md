# AI usage, HW5

## Tools

- Claude Code (model: Claude Opus 5.5), in VS Code.

## What Claude was used for

For HW5 the user wrote `lock.c` and used Claude to explain, check, and point
out mistakes. At the user's request, Claude wrote the supporting files
(Makefile, tests, sample input, main) for the user to review. Where Claude
wrote code, it is listed below.

- **Reviewing the paper table.** The user's first table on paper had six
  states (including OPENED and CLOSED) and one column per digit. Claude
  compared it with the spec: the spec fixes four states and four events
  (RIGHT, WRONG, CLOSE, TIMEOUT), CLOSE while typing should stay in the same
  state, and every cell needs an action. Claude then gave its own 16 entries
  for the user to compare with their redo.
- **Explaining concepts.**
  - Why 3 3 1 4 does not open (the second 3 is WRONG in GOT1), with a
    trace, and which entry would change that.
  - Why the types cannot be opaque here: the spec fixes them in `lock.h`,
    C11 has no forward-declared enums, and `lock_table` needs a complete
    `transition_t`.
  - Why the types are not defined again in `lock.c`.
  - Where `event_name` and `action_name` come from in the spec.
  - How the files work together: `lock_event`, then `lock_step`, then the
    name functions, with `main` holding the state.
  - That `lock_step` changes nothing itself; `main`'s `state = next` does.
  - The difference between the array index (the state) and the value (the
    digit) in `code[]`, and between `'3'` and `3`.
- **`lock_step`.** The user's first versions looped over the table with an
  `if`. Claude explained that the loop only ever matches `lock_table[s][e]`
  and showed one line, `return lock_table[s][e].next;`. The user wrote the
  action line and removed the loops.
- **`lock_event`.** Claude described the structure (digit, then `c`, then
  `t`, then -1) and proposed the `code[]` array indexed by the state, and the
  comparison `c == code[s]`, instead of one `if` per state. Claude pointed out
  the problems in each version (`'CLOSE'` as a multi-character constant, a
  stray brace, the two conditions swapped), and the user fixed them.
- **Name functions.** Claude explained the array and the range check and
  wrote `state_name` as an example. The user wrote all three, with their own
  range check.
- **Reviewing the user's code.** Claude compiled `lock.c` with
  `gcc -std=c11 -Wall -Wextra` after each change and checked it with
  throwaway programs outside the repo. It found six table entries that did
  not match the README, including GOT2 + CLOSE going to OPEN (3 1 c opened
  the door), and the `E_`/`A_` prefixes in the name strings.
- **Code Claude wrote**, each at the user's request:
  - the skeleton of `lock.c` (the empty table rows and the function stubs)
    and the `event_name` and `action_name` prototypes in `lock.h`;
  - the empty deliverable files, including `check.c`;
  - `Makefile`, in the same layout as HW4, and the two `hw05/` lines in
    `.gitignore` for the built programs;
  - the comments on the three name functions in `lock.h`;
  - `data/codes.txt` and `tests/test_lock.c` (33 tests: the 16 entries
    against a separate expected table, `lock_event` in each state, and the
    four sequences), for the user to review;
  - `main.c`: the user agreed on the plan (skip non-events silently) and
    asked Claude to write the draft for review;
  - `check.h`: Claude moved the user's `fsm_check` prototype there from
    `lock.h`, where it did not compile without `<stdio.h>`.
- **Bonus.** The user started `check.c` with four helpers that called
  `lock_step`. Claude reviewed it: the helpers read the real `lock_table`
  instead of the `table` parameter, printed to stdout instead of `out`,
  returned nothing, the unreachable check was not a search, and the dead-end
  check tested for undefined entries. At the user's request, Claude then made
  all of those corrections, keeping the user's helper names and comments, and
  wrote `fsm_check`, the breadth-first search, `tests/test_check.c`, the
  `make bonus` target, the `.gitignore` line, and a draft of `BONUS.md`.
- **Documentation.** Claude wrote this file and `README.md`: the transition
  table in ASCII, the state diagram copied in ASCII from the user's drawing,
  and the build, main, and test sections.

## Done by the user, not by AI

- The paper transition table, its redo, and the state diagram.
- `lock.h`: the types and prototypes, typed from the assignment, and the
  comments on each line.
- `lock_table`: all 16 entries.
- `lock_step`, `lock_event`, and the three name functions in `lock.c`, with
  the help described above.
- The labels added to the diagram: FAIL on LOCKED's self-loop, the
  GOT1 → LOCKED label, NEXT on OPEN's self-loop, and NEXT/UNLOCK.
- Reviewing and editing the files Claude wrote.

## Something the model got wrong

- **The range check in the name functions.** Claude said the user's
  `s >= 0 && s < N_STATES` would cause a warning under `-Wextra` and
  suggested a `(unsigned)` cast instead. It does not: the user's version
  compiles with no warnings and returns `"?"` for -1. Claude corrected this
  after testing it.
- **A misleading "compiled clean".** One of Claude's compile checks printed
  `COMPILED_CLEAN` even though gcc had printed three warnings, because gcc
  still exits 0 on warnings. Claude reported the warnings, but the marker
  itself was wrong, and later checks show the full gcc output instead.
