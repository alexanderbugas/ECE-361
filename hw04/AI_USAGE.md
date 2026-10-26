# AI usage, HW4

## Tools

- Claude Code (model: Claude Opus 5.5), in VS Code.

## What Claude was used for

For HW4 the user writes the code and uses Claude mainly to explain, check, and
point out mistakes, so that the user can walk through their own code in the
Shine. Where Claude wrote code, it is listed below.

- **Reading compiler and editor errors.** Claude explained the errors VS Code
  and gcc reported: the missing `<stdio.h>` that `FILE` needs in `tally.h`,
  `#include <stdin.h>` in `events.c` (the header is `<stdio.h>`), a missing
  `;` after `struct entry`, which the compiler reported on the line after it,
  and the errors in early versions of `tally_create` and `tally_size`.
- **Checking against the spec.** After the user shared the assignment, Claude
  compared the files with it and listed what was missing or did not match
  (the struct layout, the pool, per-function comments in `tally.h`, the
  `tests/` folder, the files still to write).
- **Designing the struct.** Claude proposed `struct entry` (a name and a
  count) and a `struct tally` holding an array of entries, a name counter, and
  an in-use flag, plus the `static` pool. The user typed them in and chose the
  names `num_names` and `datapool`.
- **Explaining concepts.** Why `struct entry` exists, what is inside
  `char name[16]`, how a plain array stays sorted, why the shift in
  `tally_add` starts at the end, why `tally_create` cannot return a local
  variable, `.` vs `->`, `=` vs `==`, and why `sizeof` is not the count.
- **Pseudocode.** At the user's request, Claude wrote high-level pseudocode
  for every function, with questions to think about, for the user to turn
  into code.
- **Reviewing the user's code.** Claude reviewed each version of
  `tally_create`, `tally_destroy`, `tally_size`, and `tally_count`, traced
  them by hand, and compiled them with `gcc -std=c11 -Wall -Wextra`.
- **Code Claude wrote**, each at the user's request:
  - the NULL check in `tally_destroy`;
  - the final `tally_size` (the `== NULL` check returning 0, and
    `return t->num_names;`), after the user's version tried to assign a
    `sizeof` result to the struct;
  - `tally_count`, rewritten from the user's started version with the fixes
    Claude had listed (NULL check for both arguments, loop to `num_names`,
    `strcmp(...) == 0` inside an `if`, `return 0` after the loop).
  - the body of the search loop in `tally_add` (Phase 2): the user wrote
    the Phase 1 input checks, declared `i` before the loop and set the loop
    bound; Claude filled in the `strcmp` result, the match case
    (`count++` and return it), and the `break` when `name` comes first.
  - the code for the rest of `tally_add` (the full-tally check, the shift
    loop, and the four lines that fill the new entry), shown in the chat at
    the user's request after the user had trouble with the syntax. The user
    typed it into `tally.c` and added the comments.
- **`events.c`.** The user wrote the outline: the includes, `main`, the
  `tally_create` call, the buffer, the `fgets` loop, the newline removal
  (with `strlen`, which the user chose over `strcspn`), and the comments for
  each step. At the user's request, Claude then fixed it so it compiled
  (`int main(void)`, the NULL check after `create`, the `skipped` counter)
  and finished the loop and the end: `continue` instead of `return` for blank
  and `#` lines, checking only `input[0]` for `#`, the `tally_add(...) == -1`
  check, and the print, summary line and destroy after the loop.
- **Makefile.** Claude wrote `hw04/Makefile` at the user's request, in the
  same layout as HW3: `make` builds `events`, `make test` builds and runs
  `tests/test_tally`, `make clean` removes what was built, and each `.o` lists
  the headers it includes. Claude also added `hw04/events` and
  `hw04/tests/test_tally` to `.gitignore`.
- **Tests.** Claude moved `test_tally.c` into `tests/` and wrote the tests at
  the user's request, keeping the user's list of required cases at the top.
  There is one test function for each case the assignment lists, each with a
  comment on what it checks and why. The user reviews them and will update
  them once the code is finished.
- **Header comments.** At the user's request, Claude added `#include
  <stdio.h>` to `tally.h` (for `FILE`) and wrote the comment for each
  function (takes, returns, assumes), based on the assignment's table. Claude
  also removed the unused `#include <stdlib.h>` from `tally.c` and replaced
  the misplaced comment above `struct tally`.
- **Documentation.** Claude wrote this file and `README.md`, at the user's
  request.

## Done by the user, not by AI

- `tally.h`: the interface and constants, typed from the assignment.
- `tally_create`: the user typed every version. Claude pointed out the
  problems in each and described the fix in words (check for a free slot,
  return `&datapool[i]`, `return NULL` after the loop), and the user wrote
  the code.
- `tally_print`: written by the user. Claude explained `fprintf` and that
  `out` is an open stream, and pointed out the loop bound errors
  (`num_names` without `t->`, then an extra `- 1` that skipped the last
  name, which the print-order test showed).
- `data/door.txt`: the sample input.
- Part 3: the user added the `t->num_names` line to `events.c`, ran `make`,
  and wrote the paragraph in the README. Claude pointed out that the first
  attempt was above the declaration of `t`, which gave `'t' undeclared`
  instead of the opaque-type error, and pasted the output into the README.
- The body of `tally_destroy` (`t->in_use = 0;`), and the decision, after
  Claude's explanation, not to clear the entries.
- The comment headers in `events.c` and `test_tally.c`, and the per-function
  comments in `tally.c`.
- Choosing to do the work in small steps and asking for explanations and
  pseudocode rather than finished code.

## Something the model got wrong

- **`malloc` in `tally_create`.** Before the user shared the assignment,
  Claude's first example of `tally_create` used `malloc(sizeof *t)`. The
  assignment says "No malloc (that is week 7)": the tallies must live in a
  `static` array of `TALLY_MAX`, each marked in use or free. Claude
  corrected this once it had the assignment, and the code uses the pool.
- **The first struct layout did not match the spec.** In the same early
  answer, Claude suggested two parallel arrays, `names[TALLY_NAMES][...]` and
  `counts[TALLY_NAMES]`. The assignment asks for "an array of entries, each a
  name and a count". Claude replaced it with `struct entry`.
- **Overstepping its role (in the user's words).** The AI sometimes tries to
  build scaffolding by predicting the structure implicit in certain function
  creations because it predicts those functions will be dependent on data
  (not usually user input but stored somewhere). Effective prompting prevents
  it from overstepping its role, creating functionality that was not asked
  for, or providing excess word vomit that pollutes the conversation in a
  distracting way. It also makes it provide more elegant and intuitive coding
  solutions that plug in a helpful tip to existing user written code without
  letting its contributions steer the direction of the code structure too
  much.
