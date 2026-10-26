# HW4: The tally ADT

A **tally** counts how often each name occurs, such as the events a device
sees. It is an abstract data type: callers such as `events.c` use it only
through `tally.h`, and only `tally.c` knows what is inside a `tally_t`.

## How to build and test

Run these from `hw04/`:

```bash
make            # builds events with gcc -std=c11 -Wall -Wextra, no warnings
make test       # builds and runs tests/test_tally: PASS or FAIL per test, then a summary
make clean      # removes everything make built
./events < data/door.txt    # tallies the names in a sample input
```

`make test` exits nonzero if any test fails. Each `.o` in the Makefile lists
the headers its `.c` includes, so changing `tally.h` rebuilds everything that
uses it. The tests are linked with the same `tally.o` as `events`.

With the sample input in `data/door.txt`, `./events < data/door.txt` prints:

```
open 3
smoke 2
steam 1
names: 3, lines skipped: 1
```

The `# warning` line is skipped without being counted, and
`absolutely_not_checking` (23 characters, more than `TALLY_LEN`) is the one
line `tally_add` refused.

## What the tests check

`tests/test_tally.c` includes only `tally.h`, like `events.c`, so it uses a
tally only through its six functions. All tests share the one pool inside
`tally.c`, so every test destroys each tally it creates before it returns.

| Test | What it checks, and why |
|---|---|
| counting up | adding `door` three times returns 1, 2, 3, and the tally still holds one name: a repeat lands in the existing entry |
| never added | count is 0 in an empty tally and among other names |
| empty name | `""` is refused and nothing is stored |
| name length | exactly `TALLY_LEN` characters is accepted and stored whole; `TALLY_LEN + 1` is refused. This is the edge of the length check: `>=` instead of `>` would refuse the first |
| NULL arguments | every function returns its spec value for NULL, and `destroy` and `print` do not crash |
| full tally | with `TALLY_NAMES` names, a new name is refused and changes nothing, but an existing one still counts, which is why the full check comes after the search |
| `TALLY_MAX` tallies | `TALLY_MAX` creates succeed and one more returns NULL |
| create after destroy | the freed slot is handed out again, and it starts empty even though the old names are still in memory |
| independent | two tallies do not see each other's names, which catches code that uses the pool instead of `t` |
| print order | scrambled input (front, middle and end inserts, a repeat, and `Zed`) prints in `strcmp` order, read back from a temporary file. `Zed` comes first because uppercase sorts before lowercase |

## How the tally is stored

*Written by Claude at the user's request, from the explanations in the chat.
The code itself is the user's except where `AI_USAGE.md` says otherwise.*

```c
struct entry {
    char name[TALLY_LEN + 1];   /* up to 15 characters, then '\0' */
    int  count;                 /* how many times this name was added */
};

struct tally {
    struct entry entries[TALLY_NAMES];  /* kept sorted by name */
    int num_names;                      /* how many entries are filled */
    int in_use;                         /* 1 = handed out, 0 = free */
};

static struct tally datapool[TALLY_MAX];    /* the pool: the only global */
```

- **Why `struct entry`:** a name and its count belong together. With two
  separate arrays (`names[]` and `counts[]`), every move during an insert would
  have to be done twice, and forgetting one would pair a count with the wrong
  name. With entries, `t->entries[i + 1] = t->entries[i];` moves both at once,
  because C copies a whole struct with `=`, including the `char` array inside.
- **What is in `name`:** 16 bytes. The characters of the name, then the null
  character `'\0'` that marks where the string ends. Anything after the `'\0'`
  is ignored by `strcmp`, `strlen` and `printf("%s")`.

  ```
  index:  0    1    2    3    4    5   ...  15
        +----+----+----+----+----+----+---+----+
  name: | d  | o  | o  | r  | \0 | ?  |...| ?  |
        +----+----+----+----+----+----+---+----+
                              ^ the string ends here
  ```
  A name of exactly 15 characters (`TALLY_LEN`) fills indexes 0 to 14 and the
  `'\0'` goes in 15. A 16-character name would not fit, so `tally_add` must
  check the length before it copies anything.
- **`'\0'` vs `NULL`:** `'\0'` is the null *character* that ends a string.
  `NULL` is a *pointer* that points nowhere, such as what `tally_create`
  returns when the pool is full.
- **Two different counters:** `num_names` counts names inside one tally
  (0 to 16). `in_use` says whether this tally is taken in the pool (0 or 1).
- **Why a pool and not a local variable or `malloc`:** a `struct tally`
  declared inside `tally_create` would disappear when the function returns,
  leaving the caller a dangling pointer (gcc warns `function returns address
  of local variable`). `malloc` is week 7. So all `TALLY_MAX` tallies exist in
  `datapool` for the whole run of the program, and `create` hands out a free
  one. Because `datapool` is at file scope, C sets it to all zeros at start,
  so every `in_use` starts at 0 (free). `static` keeps it private to
  `tally.c`.
- **`.` vs `->`:** `.` when you have the struct itself, `->` when you have a
  pointer to it. `datapool[i]` is a struct, so `datapool[i].in_use`. `t` is a
  pointer, so `t->in_use` (shorthand for `(*t).in_use`). In
  `t->entries[i].name`, `t` is a pointer (`->`), `t->entries[i]` is one
  `struct entry` (`.`), and `.name` is its `char` array.

  ```
     t                         datapool[2]
   +----------+           +-------------------+
   | address  | --------> | entries[...]      |
   | of [2]   |           | num_names = 3     |
   +----------+           | in_use    = 1     |
                          +-------------------+
  ```
  Every function except `create` gets a `tally_t *`, for two reasons:
  `events.c` cannot hold a `struct tally` itself (it does not know its size),
  and `tally_add` must change the real tally in the pool, not a copy.

## How each function works, and why

### `tally_create(void)`

- Walks `i` from 0 to `TALLY_MAX - 1` and looks for a slot with
  `in_use == 0`.
- On the first free slot: sets `in_use = 1`, sets `num_names = 0`, and returns
  `&datapool[i]`, the address of that slot.
- A taken slot is not a failure, so the loop just moves to the next one. Only
  after the loop, when all slots were taken, does it return `NULL`.
- **Walking it, 5 calls in a row:**

  ```
  call 1: slot 0 free             -> take it, return &datapool[0]
  call 2: slot 0 taken, skip;
          slot 1 free             -> return &datapool[1]
  call 3:                         -> return &datapool[2]
  call 4:                         -> return &datapool[3]
  call 5: all 4 taken, loop ends  -> return NULL
  ```
- **Why `num_names = 0` is enough to empty it:** a reused slot still has the
  old names in memory, but every other function only looks at
  `entries[0]` to `entries[num_names - 1]`. With `num_names` at 0, nothing old
  is ever read, and `tally_add` overwrites entries as it fills them.

### `tally_destroy(tally_t *t)`

- If `t` is NULL, returns right away: "NULL does nothing." The check comes
  before `t->in_use`, because following a NULL pointer crashes the program.
- Otherwise sets `t->in_use = 0`, so `create` can hand the slot out again.
- **Why it does not clear the entries:** `create` resets `num_names` when it
  hands the slot out, and nothing reads past `num_names`. Clearing would be
  wasted work. (Also, `t->entries[i].name = 0;` would not compile: you cannot
  assign to a whole array in C.)

  ```
  destroy:  in_use = 0                -> slot is free again
  create:   finds in_use == 0         -> takes it, sets num_names = 0
            old entries still in memory, but nobody looks at them
  ```

### `tally_count(const tally_t *t, const char *name)`

- If `t` or `name` is NULL, returns 0. (`strcmp(NULL, ...)` would crash.)
- Walks the filled entries, `i` from 0 to `num_names - 1`, and compares each
  name with `strcmp`. `strcmp` returns 0 when the two strings are identical;
  `==` on two strings would only compare their addresses.
- On a match, returns that entry's `count` right away. If the loop finishes
  with no match, the name was never added: returns 0.
- It is a search, not a sum: the count is already stored in the entry, and
  `tally_add` keeps it up to date.
- **Walking it:**

  ```
  t:   [0] "alarm"  count 2
       [1] "door"   count 5
       [2] "window" count 1        num_names = 3

  tally_count(t, "door"):
     i=0: strcmp("door", "alarm") != 0  -> keep looking
     i=1: strcmp("door", "door")  == 0  -> return entries[1].count -> 5

  tally_count(t, "motion"):
     i=0 no, i=1 no, i=2 no             -> loop ends -> return 0
  ```

### `tally_size(const tally_t *t)`

- If `t` is NULL, returns 0. Otherwise returns `t->num_names`.
- **It only reads:** the count is already in the struct. `create` sets it to
  0 and `add` adds 1 for each new name, so `size` just hands it back with
  `return`. The `const` in `const tally_t *t` makes the tally read-only here,
  so assigning to `t->num_names` would not even compile.
- **Why not `sizeof`:** `sizeof(t->entries)` is the size of the whole array
  in bytes, the same whether the tally holds 0 names or 16. It measures the
  space, not how much of it is used.
- **`==`, not `=`:** `if (t = NULL)` would set `t` to NULL, the `if` would be
  false, and `return t->num_names` would then crash on every call.

### `tally_add(tally_t *t, const char *name)`

It works in four phases:

1. **Reject bad input** (return -1, change nothing): `t` or `name` is NULL,
   `name` is empty, or `name` is longer than `TALLY_LEN`. The NULL checks come
   first, because the length checks read `name`.
2. **Search** the filled entries with `strcmp`. An exact match: add 1 to its
   count and return the new count. An entry greater than `name`: stop, this
   index is where `name` goes. No stop: it goes at the end.
3. **Check for room:** if the name is new and `num_names == TALLY_NAMES`,
   return -1. This comes after the search, because in a full tally an
   existing name must still count.
4. **Insert:** shift the entries from the insert index up by one, starting
   with the last one; copy `name` in (safe now, because the length was
   checked); set its count to 1; add 1 to `num_names`; return 1.

**Which path a call takes:** a repeat never reaches Phase 4. The search finds
it, adds 1, and returns right there, so `count = 1` runs only the first time a
name is added, when 1 is the right count. (Line numbers are in `tally.c`.)

```
                     tally_add(t, name)
                            |
                   Phase 2: search the entries
                     /                   \
         name IS already here         name is NOT here
                |                            |
       count++  (line 74)            Phase 3: full? -> -1
       return count  (line 75)              |
                                     Phase 4: shift, copy,
         (2nd, 3rd, 4th... time)       count = 1, return 1
                                        (1st time only)
```

**How every path ends:** each one ends in a `return`, so gcc has no
"control reaches end of non-void function" warning.

```
bad input         -> return -1            (Phase 1)
name exists       -> return new count     (Phase 2)
new, tally full   -> return -1            (Phase 3)
new, room         -> return 1             (Phase 4b)
```

**The outline the code was written from:** two separate loops, a search and
then a shift. `i` is declared before the first loop so it still holds the
insert spot after the loop ends; the shift uses its own `j`.

```c
int i;                                      /* declared OUT here, so it survives the loop */

/* LOOP 1 - search: walk the filled entries, compare with strcmp */
for (i = 0; i < t->num_names; i++){
    /* match?           -> count goes up by 1, return the new count */
    /* name comes first?-> break  (i is now the insert spot)        */
}
/* here: i is the insert spot (where you broke, or num_names if never) */

/* PHASE 3 - full? -> return -1 */

/* LOOP 2 - shift: from the end down to i, each entry moves up one */
for (/* start at num_names */; /* while > i */; /* count down */){
    /* entries[destination] = entries[destination - 1]; */
}

/* fill slot i: copy name, count = 1, num_names++, return 1 */
```

**How a plain array stays sorted:** nothing sorts it. It starts empty (which
is sorted), and every insert puts the new name where it belongs, so it is
always sorted. That is why `tally_print` never sorts.

**Walking an insert:** `tally_add(t, "motion")` on a tally holding 3 names.

```
search:   [0] "alarm"   < "motion"  keep going
          [1] "door"    < "motion"  keep going
          [2] "window"  > "motion"  stop: insert at 2
          [3] (unused)

shift:    [2] --copy--> [3]          last one first
          now [2] and [3] both hold "window"; [2] is free to overwrite

insert:   [0] "alarm"  2
          [1] "door"   5
          [2] "motion" 1             <- new
          [3] "window" 1             num_names = 4, return 1
```

**Why the shift starts at the end:** going front to back overwrites an
entry before it has moved. With 5 entries and an insert at index 1, copying
`[1] -> [2]` first destroys the old `[2]` before it can move to `[3]`, and
every later slot ends up a copy of `[1]`. Going back to front, each copy goes
into a slot that is empty or has already moved, so nothing is lost.

**Print order:** `strcmp` compares character codes (ASCII), so all
uppercase letters come before all lowercase ones: `"Window"` sorts before
`"alarm"`.

### `tally_print(const tally_t *t, FILE *out)`

- If `t` or `out` is NULL, returns without printing.
- Otherwise walks the filled entries, `i` from 0 to `num_names - 1`, and
  prints each as `name count` on its own line with
  `fprintf(out, "%s %d\n", ...)`. Each `%` placeholder is filled by the next
  value, in order; the space between them is the space in each line
  (`dog 2`), and `\n` ends the line:

  ```
  fprintf(out, "%s %d\n", t->entries[i].name, t->entries[i].count);
                │  │      │                   │
                │  │      └─ 1st value        └─ 2nd value
                │  └── %d gets the 2nd value (an int)
                └───── %s gets the 1st value (a string)
  ```
- **No sorting:** `tally_add` already keeps the entries in `strcmp` order.
- **`out` is a stream, not a file name:** the caller opens it and passes it
  in, such as `stdout` in `events.c` or a temporary file in the tests.
  `fprintf` is `printf` with the stream as its first argument, so the same
  function can print to the screen or into a file without knowing which.
- **Only the filled entries, only name and count:** anything past
  `num_names` is empty or left over, and `num_names` and `in_use` are
  bookkeeping, not data.
- **The loop bound is `i < t->num_names`, with no `- 1`:** `<` already stops
  at the last index. With 7 names, `i < 7 - 1` stopped at `i = 5` and the
  last name was never printed. The print-order test caught exactly this.

  ```
  t:  [0] "Zed" 1    -> Zed 1
      [1] "ant" 1    -> ant 1
      ...
      [6] "owl" 1    -> owl 1        num_names = 7, i = 0..6
  ```

## What the opaque type prevents (Part 3)

`events.c` includes only `tally.h`. To test the opaque type, this line was
added to its `main`, after `t` is created and checked for NULL:

```c
printf("%d\n", t->num_names);
```

Running `make` gave:

```
gcc -std=c11 -Wall -Wextra -g -c events.c -o events.o
events.c: In function ‘main’:
events.c:25:21: error: invalid use of incomplete typedef ‘tally_t’ {aka ‘struct tally’}
   25 |     printf("%d\n", t->num_names);
      |                     ^~
make: *** [Makefile:21: events.o] Error 1
```

The line was then removed, and `events.c` builds again with no warnings.

The opaque datatype prevented me from accessing the struct members directly
because they are defined in a file that is not visible to events.c. This is
useful because it prevents mismanagement of data that is outside the scope of
the purpose of the file I am working in. Management of the struct members is
done opaquely to protect inadvertent changes to data structures.
