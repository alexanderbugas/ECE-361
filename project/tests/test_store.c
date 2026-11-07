/*
 * test_store.c: unit tests for store.h / store.c
 *
 * The sheet's store tests, except height (week 8, when the tree exists).
 */
#include <stddef.h>
#include "check.h"
#include "store.h"

/* a record with only what the tests look at: the temperature and a tick */
static record_t make_record(float temp_c, time_t t) {
    record_t r = { .reading = { .t = t, .temp_c = temp_c, .rh_pct = 50.0f },
                   .state = CLOSED, .status = 0 };
    return r;
}

/* collects the records a visit sees, in order, for the tests to look at */
struct seen {
    record_t records[STORE_MAX];
    int n;
};

static void remember(const record_t *r, void *ctx) {
    struct seen *seen = ctx;
    seen->records[seen->n++] = *r;
}

static struct seen seen;    /* static: too big for the stack */

/*
 * record_compare: the sheet's three cases, the boundary from both sides, and
 * one pair below 0 C
 *
 * Why: a value exactly on a boundary starts the step above (SPEC.md), and
 * the key must round down for negative temperatures too.
 */
static void test_compare(void) {
    record_t a, b;

    a = make_record(21.2f, 0); b = make_record(21.4f, 0);
    check(record_compare(&a, &b) == 0, "store: 21.2 and 21.4 are the same step");
    a = make_record(21.4f, 0); b = make_record(21.6f, 0);
    check(record_compare(&a, &b) < 0, "store: 21.4 is in the step below 21.6");
    a = make_record(21.5f, 0); b = make_record(21.4f, 0);
    check(record_compare(&a, &b) > 0, "store: boundary 21.5 is above 21.4");
    a = make_record(21.5f, 0); b = make_record(21.9f, 0);
    check(record_compare(&a, &b) == 0, "store: boundary 21.5 is in the step of 21.9");
    a = make_record(-0.2f, 0); b = make_record(0.2f, 0);
    check(record_compare(&a, &b) < 0, "store: -0.2 and 0.2 are different steps");
    check(store_key(21.4f) == 42 && store_key(21.5f) == 43 && store_key(-0.1f) == -1,
          "store: keys of 21.4, 21.5 and -0.1 are 42, 43, -1");
}

/*
 * Insert into an empty store, count after 1, 2 and 100, destroy an empty one
 */
static void test_insert_count(void) {
    store_t *s = store_create();
    record_t r = make_record(25.0f, 0);

    check(s != NULL && store_count(s) == 0, "store: a new store is empty");
    check(store_insert(s, &r) == 0 && store_count(s) == 1, "store: insert into an empty store, count 1");
    r = make_record(22.0f, 1);
    store_insert(s, &r);
    check(store_count(s) == 2, "store: count after 2 inserts");
    for (int i = 2; i < 100; i++) {
        r = make_record(20.0f + (i % 17) * 0.3f, i);
        store_insert(s, &r);
    }
    check(store_count(s) == 100, "store: count after 100 inserts");
    check(store_height(s) == -1, "store: height is -1 until the tree (week 8)");
    store_destroy(s);

    s = store_create();
    store_destroy(s);
    check(1, "store: destroy an empty store");
    store_destroy(NULL);
    check(1, "store: destroy NULL does nothing");
}

/*
 * Insert duplicates and find every one of them, in the order they arrived;
 * find a key that is not there
 *
 * Why: the sheet requires it, and it is why a count was not used.
 */
static void test_duplicates(void) {
    store_t *s = store_create();
    record_t r;

    float temps[] = { 30.1f, 22.0f, 30.4f, 30.0f, 31.0f, 30.2f, 30.3f };
    for (int i = 0; i < 7; i++) {
        r = make_record(temps[i], 100 + i);
        store_insert(s, &r);
    }

    seen.n = 0;
    int n = store_find(s, store_key(30.0f), remember, &seen);
    int in_order = n == 5 && seen.n == 5;
    time_t want[] = { 100, 102, 103, 105, 106 };    /* 30.1 30.4 30.0 30.2 30.3 */
    for (int i = 0; in_order && i < 5; i++) {
        if (seen.records[i].reading.t != want[i]) {
            in_order = 0;
        }
    }
    check(n == 5, "store: find returns all 5 records with key 60");
    check(in_order, "store: duplicates come back in the order they were inserted");
    check(store_find(s, store_key(40.0f), remember, &seen) == 0, "store: find a key that is not there gives 0");
    check(store_find(s, store_key(22.0f), NULL, NULL) == 1, "store: find with no visit only counts");
    store_destroy(s);
}

/*
 * The in-order walk visits records in key order
 */
static void test_walk(void) {
    store_t *s = store_create();
    float temps[] = { 26.0f, 19.5f, 33.2f, 19.5f, -3.0f, 26.4f };
    record_t r;

    for (int i = 0; i < 6; i++) {
        r = make_record(temps[i], i);
        store_insert(s, &r);
    }
    seen.n = 0;
    store_walk(s, remember, &seen);

    int sorted = seen.n == 6;
    for (int i = 1; sorted && i < seen.n; i++) {
        if (record_compare(&seen.records[i - 1], &seen.records[i]) > 0) {
            sorted = 0;
        }
    }
    check(sorted, "store: the walk visits records in key order");
    check(seen.n == 6 && seen.records[0].reading.temp_c == -3.0f, "store: the walk starts at the lowest key");
    store_destroy(s);
}

/*
 * A full store refuses an insert and keeps what it has; NULL is refused
 */
static void test_full_and_null(void) {
    store_t *s = store_create();
    record_t r = make_record(25.0f, 0);
    int all_in = 1;

    for (int i = 0; i < STORE_MAX; i++) {
        r = make_record(10.0f + i * 0.01f, i);   /* increasing: appends */
        if (store_insert(s, &r) != 0) {
            all_in = 0;
        }
    }
    check(all_in && store_count(s) == STORE_MAX, "store: STORE_MAX records fit");
    check(store_insert(s, &r) == -1 && store_count(s) == STORE_MAX, "store: a full store refuses an insert");
    check(store_insert(NULL, &r) == -1 && store_insert(s, NULL) == -1, "store: insert refuses NULL");
    check(store_count(NULL) == 0, "store: count of NULL is 0");
    store_destroy(s);
}

/*
 * The pool: STORE_POOL stores at most, and a destroyed one can be reused
 */
static void test_pool(void) {
    store_t *s[STORE_POOL];
    int all = 1;

    for (int i = 0; i < STORE_POOL; i++) {
        s[i] = store_create();
        if (s[i] == NULL) {
            all = 0;
        }
    }
    check(all && store_create() == NULL, "store: no more than STORE_POOL stores");
    store_destroy(s[0]);
    s[0] = store_create();
    check(s[0] != NULL && store_count(s[0]) == 0, "store: a destroyed store can be created again, empty");
    for (int i = 0; i < STORE_POOL; i++) {
        store_destroy(s[i]);
    }
}

void test_store(void) {
    test_compare();
    test_insert_count();
    test_duplicates();
    test_walk();
    test_full_and_null();
    test_pool();
}
