/*
 * test_tally.c: unit tests for the tally ADT, ECE 361 HW4
 *
 * Prints PASS or FAIL for each test and a summary.
 * Exits with 1 if any test fails, so that make test fails.
 *
 * Like events.c, this file includes only tally.h, so it can use a tally only
 * through the six functions. It never looks inside a tally_t.
 *
 * All tests share the one pool of TALLY_MAX tallies inside tally.c, so every
 * test destroys each tally it creates before it returns. Otherwise a later
 * test could find the pool full and fail for the wrong reason.
 */

/*
Tests every function, at least:
-counting up;
-a name never added;
-an empty name;
-a name of exactly TALLY_LEN characters and one of TALLY_LEN + 1;
-NULL arguments;
-a full tally (new names refused, existing one still counts);
-TALLY_MAX tallies;
-one more (NULL);
-create after destroy;
-two tallies are independent;
-the print order;

-PASS or FAIL per test;
-a summary;
-a nonzero exit status if any tes fails;
*/

#include <stdio.h>
#include <string.h>
#include "tally.h"

static int passed = 0;
static int failed = 0;

/* record one result: cond is nonzero if the test passed */
static void check(int cond, const char *name) {
    if (cond) {
        printf("PASS  %s\n", name);
        passed++;
    } else {
        printf("FAIL  %s\n", name);
        failed++;
    }
}

/*
 * The same name added three times returns 1, 2, 3, and the tally still holds
 * only one name
 *
 * Why: the return value is the name's new count, and a repeat must land in
 * the existing entry (Phase 2's count++) instead of becoming a second entry.
 */
static void test_counting_up(void) {
    tally_t *t = tally_create();

    check(tally_add(t, "door") == 1, "counting up: first add returns 1");
    check(tally_add(t, "door") == 2, "counting up: second add returns 2");
    check(tally_add(t, "door") == 3, "counting up: third add returns 3");
    check(tally_count(t, "door") == 3, "counting up: count is 3");
    check(tally_size(t) == 1, "counting up: still one name");

    tally_destroy(t);
}

/*
 * A name that was never added counts 0, in an empty tally and in one that
 * holds other names
 *
 * Why: tally_count must search only the filled entries and return 0 when the
 * loop finds no match, not a leftover count from somewhere else.
 */
static void test_never_added(void) {
    tally_t *t = tally_create();

    check(tally_count(t, "door") == 0, "never added: 0 in an empty tally");

    tally_add(t, "alarm");
    tally_add(t, "window");
    check(tally_count(t, "door") == 0, "never added: 0 among other names");

    tally_destroy(t);
}

/*
 * An empty name is refused and changes nothing
 *
 * Why: the spec says "" returns -1. An empty string has '\0' at index 0, so
 * a tally that did not check for it would store a blank entry.
 */
static void test_empty_name(void) {
    tally_t *t = tally_create();

    check(tally_add(t, "") == -1, "empty name: add returns -1");
    check(tally_size(t) == 0, "empty name: tally stays empty");

    tally_destroy(t);
}

/*
 * A name of exactly TALLY_LEN characters is accepted, and one of
 * TALLY_LEN + 1 is refused
 *
 * Why: this is the edge of the length check. name[] holds TALLY_LEN + 1
 * bytes, so a TALLY_LEN-character name fills it exactly, with the '\0' in
 * the last byte. One more character would not fit, and copying it would
 * write past the array. A check written as >= instead of > would wrongly
 * refuse the first name. The names are built from TALLY_LEN, so the test
 * still checks the edge if the constant changes.
 */
static void test_name_length(void) {
    tally_t *t = tally_create();
    char fits[TALLY_LEN + 1];       /* TALLY_LEN characters + '\0' */
    char too_long[TALLY_LEN + 2];   /* TALLY_LEN + 1 characters + '\0' */

    memset(fits, 'a', TALLY_LEN);
    fits[TALLY_LEN] = '\0';
    memset(too_long, 'b', TALLY_LEN + 1);
    too_long[TALLY_LEN + 1] = '\0';

    check(tally_add(t, fits) == 1, "length: exactly TALLY_LEN is accepted");
    check(tally_count(t, fits) == 1, "length: exactly TALLY_LEN is stored whole");
    check(tally_add(t, too_long) == -1, "length: TALLY_LEN + 1 is refused");
    check(tally_size(t) == 1, "length: the refused name was not stored");

    tally_destroy(t);
}

/*
 * Every function handles NULL arguments without crashing, with the value the
 * spec gives
 *
 * Why: each function must check its pointers before following them. A
 * missing check crashes the whole test program here (segmentation fault),
 * which make test also reports as a failure.
 */
static void test_null_arguments(void) {
    tally_t *t = tally_create();
    tally_add(t, "door");

    check(tally_add(NULL, "door") == -1, "NULL: add with NULL tally returns -1");
    check(tally_add(t, NULL) == -1, "NULL: add with NULL name returns -1");
    check(tally_count(NULL, "door") == 0, "NULL: count with NULL tally returns 0");
    check(tally_count(t, NULL) == 0, "NULL: count with NULL name returns 0");
    check(tally_size(NULL) == 0, "NULL: size of NULL returns 0");

    /* these return nothing, so the test is that the program gets past them */
    tally_destroy(NULL);
    tally_print(NULL, stdout);
    tally_print(t, NULL);
    check(1, "NULL: destroy and print with NULL do not crash");

    check(tally_count(t, "door") == 1, "NULL: the real tally is unchanged");

    tally_destroy(t);
}

/*
 * A full tally refuses a new name, but an existing name still counts
 *
 * Why: this is why tally_add checks for room only after the search. If it
 * checked "full?" first, the existing name would be refused too.
 */
static void test_full_tally(void) {
    tally_t *t = tally_create();
    char name[16];

    /* fill all TALLY_NAMES entries with n00, n01, ... */
    for (int k = 0; k < TALLY_NAMES; k++) {
        snprintf(name, sizeof name, "n%02d", k);
        tally_add(t, name);
    }
    check(tally_size(t) == TALLY_NAMES, "full: holds TALLY_NAMES names");

    check(tally_add(t, "new") == -1, "full: a new name is refused");
    check(tally_size(t) == TALLY_NAMES, "full: size unchanged after the refusal");
    check(tally_count(t, "new") == 0, "full: the refused name was not stored");
    check(tally_add(t, "n00") == 2, "full: an existing name still counts");

    tally_destroy(t);
}

/*
 * TALLY_MAX tallies can exist at once, and one more is NULL
 *
 * Why: the pool has exactly TALLY_MAX slots. create must hand out each free
 * slot once, and return NULL only when all of them are taken.
 */
static void test_max_tallies(void) {
    tally_t *t[TALLY_MAX];
    int all_ok = 1;

    for (int k = 0; k < TALLY_MAX; k++) {
        t[k] = tally_create();
        if (t[k] == NULL) {
            all_ok = 0;
        }
    }
    check(all_ok, "pool: TALLY_MAX creates all succeed");

    tally_t *extra = tally_create();
    check(extra == NULL, "pool: one more create returns NULL");

    for (int k = 0; k < TALLY_MAX; k++) {
        tally_destroy(t[k]);
    }
    tally_destroy(extra);       /* NULL if the test passed: does nothing */
}

/*
 * After a destroy, create can hand the slot out again, and it starts empty
 *
 * Why: destroy must mark the slot free, and create must reset the name
 * counter. The old names are still in memory, so a create that forgot to
 * reset num_names would hand back a tally that already holds them.
 */
static void test_create_after_destroy(void) {
    tally_t *t[TALLY_MAX];

    for (int k = 0; k < TALLY_MAX; k++) {
        t[k] = tally_create();
    }
    tally_add(t[0], "door");
    tally_add(t[0], "alarm");

    tally_destroy(t[0]);        /* the pool now has one free slot */
    tally_t *again = tally_create();

    check(again != NULL, "reuse: create after destroy succeeds");
    check(tally_size(again) == 0, "reuse: the reused tally starts empty");
    check(tally_count(again, "door") == 0, "reuse: old names are gone");

    tally_destroy(again);
    for (int k = 1; k < TALLY_MAX; k++) {
        tally_destroy(t[k]);
    }
}

/*
 * Two tallies do not share names or counts
 *
 * Why: each tally is its own slot in the pool. A function that used the pool
 * directly (datapool[0] or datapool->) instead of its t argument would put
 * every name in the same tally.
 */
static void test_independent(void) {
    tally_t *a = tally_create();
    tally_t *b = tally_create();

    tally_add(a, "door");
    tally_add(a, "door");
    tally_add(b, "window");

    check(tally_count(a, "door") == 2, "independent: a counts its own name");
    check(tally_count(b, "door") == 0, "independent: b does not see a's name");
    check(tally_count(a, "window") == 0, "independent: a does not see b's name");
    check(tally_size(a) == 1 && tally_size(b) == 1, "independent: one name each");

    tally_destroy(a);
    tally_destroy(b);
}

/*
 * tally_print writes one "name count" line per name, in strcmp order
 *
 * Why: the names go in scrambled, so each one has to be inserted at the
 * front, in the middle, or at the end, and the later entries shifted up. If
 * the shift ran in the wrong direction or stopped one entry early, the order
 * or the counts printed here would be wrong. "Zed" checks that the order is
 * strcmp (ASCII) order: uppercase letters come before all lowercase ones.
 *
 * The output goes to a temporary file instead of the screen, so the test can
 * read it back and compare it with the expected text.
 */
static void test_print_order(void) {
    tally_t *t = tally_create();
    const char *in[] = {"owl", "ant", "fox", "dog", "eel", "cat", "dog", "Zed"};
    const char *expected = "Zed 1\nant 1\ncat 1\ndog 2\neel 1\nfox 1\nowl 1\n";
    char got[256] = "";

    for (int k = 0; k < 8; k++) {
        tally_add(t, in[k]);
    }

    FILE *f = tmpfile();
    if (f == NULL) {
        check(0, "print order: could not open a temporary file");
        tally_destroy(t);
        return;
    }
    tally_print(t, f);
    rewind(f);                                  /* back to the start to read it */
    size_t n = fread(got, 1, sizeof got - 1, f);
    got[n] = '\0';
    fclose(f);

    check(strcmp(got, expected) == 0, "print order: sorted by strcmp, one line per name");
    if (strcmp(got, expected) != 0) {
        printf("      expected:\n%s      got:\n%s", expected, got);
    }

    tally_destroy(t);
}

int main(void) {
    test_counting_up();
    test_never_added();
    test_empty_name();
    test_name_length();
    test_null_arguments();
    test_full_tally();
    test_max_tallies();
    test_create_after_destroy();
    test_independent();
    test_print_order();

    printf("\n%d passed, %d failed\n", passed, failed);
    return failed == 0 ? 0 : 1;
}
