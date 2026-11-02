# HW5: The combination lock

## How to build and test

*Makefile written by Claude, modeled on the user's hw04 Makefile.*

Run these from `hw05/`:

```bash
make            # builds lock with gcc -std=c11 -Wall -Wextra -g
make test       # builds and runs tests/test_lock
make clean      # removes everything make built
./lock < data/codes.txt
```

Each `.o` in the Makefile lists the headers its `.c` includes, so changing
`lock.h` rebuilds everything that uses it. The tests link the same `lock.o`
as `lock`.

## How main works

*Skips non-events silently was agreed with the user.*

- Starts in LOCKED and reads one character at a time with `getchar` until EOF.
- `lock_event` turns the character into an event; -1 (newline, space, other)
  is skipped and prints nothing.
- `lock_step` gives the next state and action; one line is printed with the
  old state, the character, the event, the next state, and the action.
- Only then does the state move to the next state.
- At EOF it prints `final: STATE`.

## What the tests check

*`tests/test_lock.c` and `data/codes.txt` written by Claude at the user's
request, for the user to review.*

| Test | What it checks |
|---|---|
| table | all 16 entries, against an expected table written separately in the test |
| event digits | a right and a wrong digit in LOCKED, GOT1, GOT2 |
| event open | every digit is WRONG while OPEN |
| event other | `c` is CLOSE and `t` is TIMEOUT in every state; newline, space, `x`, `C` are -1 |
| sequences | 3 1 4 opens with UNLOCK; 3 3 1 4 and 3 1 t 4 stay LOCKED; 3 1 4 c locks with LOCK |

`data/codes.txt` has one sequence per line. Newlines are -1 and are skipped.
It has no comments, because a `c` or `t` inside a comment would be read as an
event.

## Transition table

*Each cell is `next state, action`. The user drew the first version on paper.*

```
+---------------+--------------+--------------+--------------+--------------+
| state \ event | E_RIGHT      | E_WRONG      | E_CLOSE      | E_TIMEOUT    |
+---------------+--------------+--------------+--------------+--------------+
| LOCKED        | GOT1, NONE   | LOCKED, BEEP | LOCKED, NONE | LOCKED, NONE |
+---------------+--------------+--------------+--------------+--------------+
| GOT1          | GOT2, NONE   | LOCKED, BEEP | GOT1, NONE   | LOCKED, NONE |
+---------------+--------------+--------------+--------------+--------------+
| GOT2          | OPEN, UNLOCK | LOCKED, BEEP | GOT2, NONE   | LOCKED, NONE |
+---------------+--------------+--------------+--------------+--------------+
| OPEN          | OPEN, NONE   | OPEN, NONE   | LOCKED, LOCK | LOCKED, LOCK |
+---------------+--------------+--------------+--------------+--------------+
```

## State diagram

*The user drew this on paper; Claude copied it into ASCII. Labels are as
drawn: NEXT is a right digit, FAIL is a wrong digit.*

```
                  CLOSE, TIMEOUT, FAIL/BEEP
                           +-----+
                           |     |
                           |     v
  CLOSE, TIMEOUT/LOCK  +--------------+        NEXT
   +------------------>|    LOCKED    |------------------+
   |                   +--------------+                  |
   |                       ^      ^ FAIL/BEEP,           v
   |                       |      | TIMEOUT      +--------------+
   |                       |      +--------------|     GOT1     |---+
+--------------+           |                     +--------------+   | CLOSE
|     OPEN     |---+       | FAIL/BEEP,              |      ^       |
+--------------+   | FAIL, | TIMEOUT                 |      +-------+
   ^      ^        | NEXT  |                         |
   |      +--------+       |                         | NEXT
   |                       |                         |
   |  NEXT/UNLOCK  +--------------+                  |
   +---------------|     GOT2     |<-----------------+
                   +--------------+
                       |      ^
                       +------+
                        CLOSE
```

## Bonus: the table checker

*`check.c`, `tests/test_check.c`, and `BONUS.md` drafted by Claude at the
user's request, from the user's started `check.c` (the four helpers and their
comments), for the user to review.*

```bash
make bonus      # builds and runs tests/test_check
```

### How the checker works

`fsm_check(table, start, out)` in `check.c` calls four helpers, adds up the
problems they found, and returns the total. Each helper prints one line per
problem to `out`.

- **Reads `table`, not `lock_step`.** `lock_step` always uses the real
  `lock_table`. Reading `table[s][e]` directly lets the checker look at
  broken copies.
- **`check_undefined`:** loops over all 16 entries and reports each one whose
  action is `A_UNDEFINED`. A left-out entry is zero, `{ LOCKED, A_UNDEFINED }`,
  which is why `A_UNDEFINED` is 0.
- **`check_range`:** loops over all 16 entries and reports each next state
  outside `0` to `N_STATES - 1`, and each action outside `0` to
  `N_ACTIONS - 1`. The action check is an extra; the spec asks only for the
  next state.
- **`check_unreachable`:** breadth-first search from `start`.
  - `visited[]` marks states reached; `queue[]` holds states reached but not
    yet explored; `head` is the next one to take out, `tail` the next free
    slot.
  - Start with `start` visited and in the queue. Take a state out, look at
    the next state of each of its 4 events, and add any not yet visited.
    Repeat until the queue is empty.
  - Entries left out or out of range are skipped, so a zero entry is not
    followed as if it really went to LOCKED, and `visited[7]` is never read.
  - Every state still not visited is reported.
- **`check_dead_end`:** for each state, reports it if all 4 events lead back
  to that same state.

### How the checker is tested

`tests/test_check.c` includes `check.h` (which brings in `lock.h`) and is
linked with `check.o` and `lock.o`. Each test copies `lock_table` into a
local array `t` with `memcpy` (the real table is `const`), breaks one or two
entries, calls `fsm_check(t, LOCKED, stdout)`, and compares the count:

| Table | Problems |
|---|---|
| the real `lock_table` | 0 |
| GOT2 RIGHT left out | 2: left out, OPEN unreachable |
| GOT1 RIGHT goes to state 7 | 3: out of range, GOT2 and OPEN unreachable |
| LOCKED WRONG action 9 | 1: action out of range |
| GOT2 RIGHT stays in GOT2 | 1: OPEN unreachable (only the search finds it) |
| OPEN CLOSE and TIMEOUT stay in OPEN | 1: OPEN has no way out |
| every entry left out | 20: 16 left out, 3 unreachable, LOCKED stuck |

The checker's lines are printed under each test's heading. `BONUS.md` shows
three of them and why each is a real bug for a lock.

The checker finds tables that are not well-formed state machines, not tables
that do the wrong thing: the old GOT2 CLOSE → OPEN bug passes all four
checks. Only `tests/test_lock.c` catches that.
