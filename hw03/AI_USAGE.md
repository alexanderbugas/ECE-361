# AI usage, HW3

## Tools

- Claude Code (model: Claude Opus 5.5), in VS Code.

## What Claude was used for

For HW3 the user handed most of the writing to Claude and approved every step
before the next one started.

- **Planning.** Claude summarized the homework's objectives and intended
  outcomes, read the lecture slides to stay within what the course has covered
  (no `malloc`, no structs, no recursion in this homework), and proposed an
  order of work: header first, then the build and test scaffold, then one
  function with its tests at a time.
- **Header (`ptr.h`).** Claude wrote the four prototypes and a contract
  comment for each (takes, returns, assumes), including the edge cases the
  assignment leaves open. The user decided two of them (see below).
- **Makefile.** Claude wrote `hw03/Makefile` in the same layout as HW2: `make`
  builds `ptr.o` and `tests/test_ptr`, `make test` runs the tests, `make clean`
  removes what was built, and each `.o` lists the headers it includes.
- **.gitignore.** Claude added `hw03/tests/test_ptr`.
- **The four functions (`ptr.c`).** Claude wrote `swap_float`, `minmax`,
  `find_above`, and `reverse`, one per step, each walking the array with a
  pointer and never with `a[i]`.
- **Tests (`tests/test_ptr.c`).** Claude wrote the `check()` harness (the same
  as HW2) and every test case the assignment lists, plus one test per NULL
  decision the user made. After each function, Claude ran the tests in a
  scratch folder against deliberately broken copies of that function to
  confirm the tests can fail.
- **README.** Claude wrote `README.md` at the user's request, including a
  "how each function works, and why" section and a tests section. At the
  user's request, each function's section has a walkthrough of what happens
  when it runs (boxes and arrows, and a step-by-step table), going deeper than
  the inline comments.
- **Documentation.** Claude writes this file and keeps `README.md` up to date,
  marking which decisions were the user's.

## Done by the user, not by AI

- **Reviewed and approved every step** before the next one was written: the
  header, the scaffold, and each function with its tests.
- **NULL in `swap_float`.** The assignment does not say. The user decided it
  should be guarded like the other three functions: a NULL pointer means it
  does nothing.
- **NULL in `find_above`.** The user decided that a NULL array or `n < 1`
  returns NULL instead of crashing.
- **README walkthroughs.** The user asked for the "boxes and arrows"
  explanations to go in the README, not only in the chat.

## Something the model got wrong

The code itself was right the first time: the header, the Makefile, and all
four functions in `ptr.c` built with no warnings, and every test passed on its
first run without changes. The mistakes were in comments and in one file
operation. Claude found each of them itself, without the user pointing them
out, and fixed them before handing the step over for review.

- **A wrong reason in a test comment.** The comment on the even-length
  `reverse` test said the loop must stop at `lo < hi` because "a third swap
  would undo the middle pair." That is false: with `lo <= hi` instead, the two
  pointers still cross after two swaps on a 4-element array, so there is no
  third swap. Claude noticed this while choosing broken copies of `reverse`
  to test against, and rewrote the comment to name a bug the test really does
  catch: a loop that runs `lo` all the way to the end swaps every pair twice
  and puts the array back the way it was. The broken copy confirmed it: that
  version fails both length tests.
- **A test comment that claimed too much.** The comment on the "n < 2 and
  NULL" test said it guards against the danger of `hi = a + n - 1` with
  `n == 0`. Claude then ran the tests against a copy of `reverse` with the
  guard removed, and every test still passed: with `n == 0` the loop never
  runs, and with NULL, `swap_float`'s own guard absorbs the calls. The guard
  is still needed (computing `a - 1` is undefined behavior even if it is
  never read), but no test can see it. Claude rewrote the comment to say what
  the test checks and what it cannot, and the README says the same.
- **Deleting a file by mistake.** While setting up the scaffold, Claude
  deleted `hw03/.gitkeep`. Claude noticed it in the same step and restored it
  with `git checkout -- hw03/.gitkeep` before reporting.
