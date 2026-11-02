# HW5 bonus: the table checker

*Drafted by Claude at the user's request, for the user to rewrite. The output
below is copied from `make bonus`.*

`fsm_check(table, start, out)` in `check.c` runs four checks and returns the
number of problems, one printed line each:

- an entry left out (action `A_UNDEFINED`, which is 0);
- a next state out of range (and, as an extra, an action out of range);
- a state that cannot be reached from `start`, by breadth-first search;
- a state with no way out (every event leads back to it).

The search does not follow entries that are left out or out of range, so a
left-out entry, which is zero and so points to LOCKED, is not treated as a
real edge.

Run it with `make bonus`.

## Broken copy 1: GOT2 RIGHT left out

```c
t[GOT2][E_RIGHT] = (transition_t){ 0 };
```

```
GOT2 RIGHT: entry left out (A_UNDEFINED)
OPEN: cannot be reached from LOCKED
```

2 problems. A forgotten entry is zero, `{ LOCKED, A_UNDEFINED }`, so typing
the last digit of 3 1 4 sends the lock back to LOCKED: the right code never
opens the door. The compiler accepts the table without a warning, which is
why the checker looks for `A_UNDEFINED`.

## Broken copy 2: OPEN CLOSE and OPEN TIMEOUT stay in OPEN

```c
t[OPEN][E_CLOSE].next = OPEN;
t[OPEN][E_TIMEOUT].next = OPEN;
```

```
OPEN: no way out (every event stays in OPEN)
```

1 problem. Once the door opens, nothing can lock it again: closing the door
and the timer both leave it open. Every entry is defined and in range, and
every state is reachable, so only the no-way-out check finds it.

## Broken copy 3: GOT1 RIGHT goes to state 7

```c
t[GOT1][E_RIGHT].next = (state_t)7;
```

```
GOT1 RIGHT: next state 7 out of range
GOT2: cannot be reached from LOCKED
OPEN: cannot be reached from LOCKED
```

3 problems. After 3 1, `lock_step` returns state 7, which does not exist.
`state_name` prints `?`, and the next `lock_step` reads `lock_table[7]`,
past the end of the table, so the lock does whatever that memory says. Since
GOT2 is never reached, OPEN is not either.

## What the checker cannot find

It checks that the table is a well-formed state machine, not that it is the
right one. For example, GOT2 CLOSE going to OPEN (a bug `lock.c` had at one
point) passes all four checks, but lets 3 1 c open the door. Only the tests in
`tests/test_lock.c` catch that.
