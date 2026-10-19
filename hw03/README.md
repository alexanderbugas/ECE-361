# HW3: A pointer library

Four small functions that use pointers in the three ways the lecture covered:
changing the caller's variables (`swap_float`), returning more than one result
(`minmax`), and walking an array with pointer arithmetic (`find_above`,
`reverse`). Inside `ptr.c` every array is walked with a pointer (`p++`,
`p < a + n`, `*p`), never with `a[i]`.

## How to build and test

Run these from `hw03/`:

```bash
make            # builds ptr.o and tests/test_ptr
make test       # runs tests/test_ptr: PASS or FAIL per test, then a summary
make clean      # removes ptr.o, tests/test_ptr.o, and tests/test_ptr
```

`make test` exits nonzero if any test fails.

## How each function works, and why

*Written by Claude at the user's request. The user reviews and approves each
function before the next one is written.*

### `swap_float(float *a, float *b)`

- If `a` or `b` is NULL, it returns without doing anything.
- Otherwise: `float t = *a; *a = *b; *b = t;`
- **Why the pointers:** C passes every argument by value. A
  `swap_float(float a, float b)` would swap its own copies and the caller
  would see no change (the `swap_wrong` example from lecture). With pointers,
  `a` and `b` are arrows into the caller's frame, and `*a = ...` changes the
  box at the end of the arrow.
- **Boxes and arrows:** when `swap_float(&x, &y)` runs, its frame holds two
  boxes, `a` and `b`, each with an arrow back into the caller's `x` and `y`.
  `*a = *b` means "follow arrow `b`, read the value, follow arrow `a`, write it
  there." Without the `*`, `a` and `b` would be plain floats holding copies,
  and the swap would happen only inside that frame, which disappears when the
  function returns.

  ```
  caller:      x [ 1.5 ]     y [ -2.0 ]
                    ^             ^
  swap_float:  a [  *  ]     b [  *  ]     t [ 1.5 ]
  ```
- **Same address twice:** `swap_float(&x, &x)` leaves `x` unchanged, because
  `t` holds the old value before anything is overwritten.

### `minmax(const float *a, int n, float *min, float *max)`

- If `n < 1` or `a`, `min`, or `max` is NULL, it returns -1 before touching
  anything.
- Otherwise it starts with `lo = hi = *a`, walks `p` from `a + 1` while
  `p < a + n`, and keeps the smallest and largest `*p`.
- It writes `*min = lo` and `*max = hi` once, at the end, and returns 0.
- **Walking it, for `a = {-5.0, -9.0, -3.5, -2.0}`, `n = 4`:** the caller
  passes `&lo` and `&hi`, so `min` and `max` are arrows back into the
  caller's frame. Inside, `minmax` keeps its own local `lo` and `hi` (separate
  boxes in its own frame) and only follows the arrows at the very end.

  | Step | `p` | `*p` | local `lo` | local `hi` |
  |---|---|---|---|---|
  | start | | | -5.0 (`*a`) | -5.0 (`*a`) |
  | 1 | `a + 1` | -9.0 | -9.0 | -5.0 |
  | 2 | `a + 2` | -3.5 | -9.0 | -3.5 |
  | 3 | `a + 3` | -2.0 | -9.0 | -2.0 |
  | exit | `a + 4` | not read | -9.0 | -2.0 |

  At `a + 4`, `p < a + n` is false, so the loop ends without reading past the
  array. Then `*min = lo` follows the `min` arrow and writes -9.0 into the
  caller's variable, and `*max = hi` writes -2.0. If the guard had returned
  -1, the arrows would never have been followed, and the caller's variables
  would still hold whatever they held before.

  ```
  caller:  lo [ -9.0 ]    hi [ -2.0 ]          (written last)
                 ^              ^
  minmax:  min [ * ]      max [ * ]      a [ * ]--> -5.0 -9.0 -3.5 -2.0
  ```
- **Why start from `*a`, not from 0:** starting from 0 is the bug in the
  `max.c` example from the first lecture: for `{-5, -2, -9}` it returned 0,
  a number not in the array. The first element is always a real candidate.
- **Why return -1 instead of storing something:** with no values there is no
  correct minimum. Any number stored would look like a real answer. The return
  value tells the caller whether `*min` and `*max` mean anything.
- **Why write the outputs last:** so that a failed call is guaranteed to leave
  them unchanged.
- **Why `const float *a`:** `minmax` only reads the array. `const` makes that a
  promise the compiler enforces: `*a = 0` inside the function is a compile
  error.

### `find_above(const float *a, int n, float threshold)`

- If `a` is NULL or `n < 1`, it returns NULL.
- Otherwise it walks `p` from `a` while `p < a + n`, and returns `p` at the
  first `*p > threshold`. If the loop ends, nothing matched, and it returns
  NULL.
- The comparison is strict `>`: a value equal to the threshold does not match.
  This is the same boundary as the longest run in HW2.
- **Walking it, for `a = {20.0, 31.0, 25.0, 31.0}`, threshold 30.0:** `p`
  starts as a copy of `a`, an arrow to the first element. Each `p++` moves
  the arrow one float (4 bytes) to the right. At each stop `*p` follows the
  arrow and reads that float.

  | Step | `p` | `*p` | `*p > 30.0` | what happens |
  |---|---|---|---|---|
  | 1 | `a` | 20.0 | false | `p++` |
  | 2 | `a + 1` | 31.0 | true | `return p` |

  The function hands back the arrow itself, `a + 1`, not the 31.0. The
  caller can compare it with `&a[1]` to see *which* element matched, or
  follow it with `*` to read the value. The second 31.0 is never looked at.
  If no value matches, `p` reaches `a + n`, the loop test fails before that
  address is read, and the function returns NULL: an arrow to nothing.

  ```
  a[]:     [ 20.0 ][ 31.0 ][ 25.0 ][ 31.0 ]
                      ^
  caller:  result [ * ]          (result == &a[1])
  ```
- **Why it returns a pointer, not a value:** a pointer says *where* the match
  is, and NULL can mean "not found" without using a special float value that
  might also be a real reading.
- **Why the result is `const float *`:** the array came in read-only. If the
  returned pointer were not const, a caller could change the array through it,
  breaking the promise the parameter made.

### `reverse(float *a, int n)`

- If `a` is NULL or `n < 2`, it returns without doing anything.
- Otherwise: `lo = a`, `hi = a + n - 1`, and
  `while (lo < hi) { swap_float(lo, hi); lo++; hi--; }`
- **Walking it, for `a = {1, 2, 3, 4, 5}`, `n = 5`:** `lo` and `hi` are two
  arrows into the same array, one at each end. Each step hands both arrows to
  `swap_float`, which follows them and exchanges the two floats they point
  at. Then the arrows step toward each other.

  | Step | `lo` | `hi` | `lo < hi` | array after the step |
  |---|---|---|---|---|
  | start | `a` | `a + 4` | | `1 2 3 4 5` |
  | 1 | `a` | `a + 4` | true | `5 2 3 4 1` |
  | 2 | `a + 1` | `a + 3` | true | `5 4 3 2 1` |
  | exit | `a + 2` | `a + 2` | false | `5 4 3 2 1` |

  ```
  step 1:   [ 1 ][ 2 ][ 3 ][ 4 ][ 5 ]
              ^                   ^
              lo                  hi      swap_float(lo, hi)
  exit:     [ 5 ][ 4 ][ 3 ][ 2 ][ 1 ]
                        ^
                      lo = hi             loop ends, middle untouched
  ```

  `lo` and `hi` are already addresses, so they go into `swap_float` with no
  `&`. That is the difference from `swap_float(&x, &y)` in a caller that
  holds plain floats.
- **Why the guard comes first:** with `n == 0`, `a + n - 1` would point before
  the start of the array. Forming that pointer is undefined behavior in C,
  even if nothing is ever read through it. The guard makes sure that pointer
  is never formed.
- **Odd and even lengths:** with an odd length the two pointers meet on the
  middle element, which stays in place. With an even length they cross.
  `lo < hi` stops the loop at the right moment in both cases.
- **Why `swap_float`:** the assignment asks for it, and it is exactly the
  operation needed on each pair.

## Tests (tests/test_ptr.c)

*Harness by Claude, the same `check()` pattern as HW2.*

Each test uses a small array whose answer is worked out by hand. Comparing
floats with `==` is safe here, because the functions only copy values around
and never compute new ones. To confirm the tests can fail, each function's
tests were also run in a scratch folder against deliberately broken copies of
that function. The results are listed below.

Final checks: `make clean && make` gives no warnings, `make test` passes all
21 checks, the same tests pass when built with `-fsanitize=address`, the only
`[` in `ptr.c` is inside comments, and `make clean` leaves only the source
files.

- **swap_float**: `x = 1.5`, `y = -2.0`; after the call `x == -2.0`
  and `y == 1.5`. Then `swap_float(&x, NULL)` must leave `x` at -2.0, which
  tests the user's NULL decision. A broken copy that swaps local copies only
  (like `swap_wrong`) fails the test.
- **minmax, several values**: `{-5.0, -9.0, -3.5, -2.0}`, expecting
  min -9.0 and max -2.0. All negative, so a version that starts from 0
  reports a max of 0. The max is at the last index, so a loop that stops one
  element early reports -3.5.
- **minmax, one value**: `{23.5}` with `n = 1`. Min and max are both
  23.5. The loop starts at `a + 1`, which is already `a + n`, so the body never
  runs.
- **minmax, n = 0**: returns -1. `lo` and `hi` are set to 999 before
  the call and must still be 999 after it.
- **minmax, NULL pointer**: a NULL array returns -1 and leaves `lo`
  and `hi` at 999. A NULL `min` with a valid `max` also returns -1, and `hi`
  must stay 999: `minmax` refuses the whole call instead of filling in the
  half it can.
- **Broken copies of minmax** each fail at least one of these tests: one that
  starts from 0 (fails the several-values max and the one-value test), one
  whose loop stops at `a + n - 1` (fails the several-values max), and one that
  writes `*max` before the guard (fails all three "unchanged" checks).
- **find_above, two matches**: `{20.0, 31.0, 25.0, 31.0}` with
  threshold 30.0 must return `&a[1]`. Both matches hold 31.0, so checking the
  value alone could not tell them apart. Comparing the pointer with the
  address of the first match catches a version that returns the last one.
- **find_above, no match**: `{20.0, 25.0, 29.9}` with threshold 30.0
  returns NULL. The largest value is just below the threshold and sits at the
  last index, so the loop must read all `n` values and stop at `a + n`.
- **find_above, value equal to the threshold**: `{10.0, 30.0, 30.5}`
  with threshold 30.0 must return `&a[2]`. The 30.0 is not above 30.0. With
  `>=` the function would return `&a[1]`.
- **find_above, NULL array and n = 0**: both return NULL, which
  tests the user's decision. With `n = 0`, `a[0]` is 50.0 and would match,
  but there is nothing to look at.
- **Broken copies of find_above:** one using `>=` fails the equal-to-threshold
  test, and one that keeps walking and returns the last match fails the
  two-matches test. A third uses `p <= a + n` and reads one element past the
  end. Built with `-fsanitize=address`, it stops with "stack-buffer-overflow".
  Without the sanitizer, it failed the no-match test in our run only because
  the memory just past the array happened to hold a value above 30.0, which
  is the lecture's "one past the end, followed" point: the result depends on
  whatever is there.
- **reverse, even length**: `{1, 2, 3, 4}` must become
  `{4, 3, 2, 1}`. The pointers swap twice and then cross. Every element is
  checked, not only the ends.
- **reverse, odd length**: `{1, 2, 3, 4, 5}` must become
  `{5, 4, 3, 2, 1}`. The pointers swap twice and then meet on the middle
  element, which must still hold 3.
- **reverse, n < 2 and NULL**: `reverse(a, 1)`, `reverse(a, 0)` and
  `reverse(NULL, 2)` must leave `{1, 2}` unchanged and not crash.
- **Broken copies of reverse:** one that loops while `lo < a + n` swaps every
  pair twice and puts the array back the way it was, so it fails both length
  tests. One that starts `hi` at `a + n` reads one past the end and stops with
  "stack-buffer-overflow" under `-fsanitize=address`. One with the guard
  removed **passes every test**: with `n = 0` the loop never runs, and with
  NULL `swap_float`'s own guard absorbs the calls. The guard is still needed,
  because `a - 1` is undefined behavior, but no test can see it. The comment
  on the test says so.

## The user's decisions

- **Division of work.** For HW3 the user hands most of the writing to Claude
  and approves each step before the next: header, build and test scaffold,
  then one function with its tests at a time.
- **NULL in `swap_float`.** The assignment does not say. The user chose to
  guard it like the other three functions: if either pointer is NULL, it does
  nothing.
- **NULL in `find_above`.** If `a` is NULL or `n < 1`, it returns NULL rather
  than crashing, the same answer as "not found".
