/*
 * store: keeps every record of the run (reading, vent state, status word),
 * ordered by temperature in 0.5 C steps. Its type is opaque: only store.c
 * knows what is inside. Until trees are covered in week 8, the inside is a
 * sorted array behind the same interface.
 *
 * The key of a record is its temperature in 0.5 C steps, floorf(temp_c * 2):
 * 21.0 to 21.4 C are key 42, 21.5 to 21.9 C are key 43 (SPEC.md, "The store").
 * Records with the same key are all kept, in the order they were inserted.
 */
#ifndef STORE_H
#define STORE_H

#include <stdint.h>     /* uint32_t */
#include "sensor.h"     /* reading_t */
#include "fsm.h"        /* state_t */

#define STORE_MAX  5000     /* records one store can hold (no malloc yet) */
#define STORE_POOL 4        /* stores that can exist at the same time */

/* one record: the reading, the state at that moment, and the status word */
typedef struct {
    reading_t reading;
    state_t state;
    uint32_t status;
} record_t;

typedef struct store store_t;      /* opaque */

/* the function find and walk call once per record */
typedef void (*store_visit_t)(const record_t *r, void *ctx);

/*
 * store_key: the key of a temperature, its 0.5 C step
 * Takes:   temp_c, a temperature in degrees C
 * Returns: floorf(temp_c * 2), as an int: 21.4 gives 42, 21.5 gives 43,
 *          -0.1 gives -1
 */
int store_key(float temp_c);

/*
 * record_compare: orders two records by key
 * Takes:   a, b, two records
 * Returns: <0 if a's key is lower, 0 if the keys are equal, >0 if higher
 * Assumes: a and b are not NULL
 */
int record_compare(const record_t *a, const record_t *b);

/*
 * store_create: a new, empty store
 * Returns: the store, or NULL if STORE_POOL stores are already in use
 * Assumes: nothing. The store comes from a fixed pool inside store.c (no
 *          malloc), so give it back with store_destroy.
 */
store_t *store_create(void);

/*
 * store_destroy: gives a store back so that store_create can hand it out
 * again
 * Takes:   s, a store from store_create, or NULL (which does nothing)
 * Assumes: s is not used again afterwards
 */
void store_destroy(store_t *s);

/*
 * store_insert: adds a copy of one record
 * Takes:   s, a store; r, the record to copy in
 * Returns: 0; or -1 and nothing changes if s or r is NULL, or the store
 *          already holds STORE_MAX records
 * Assumes: the store keeps its own copy, so r may be reused afterwards
 */
int store_insert(store_t *s, const record_t *r);

/*
 * store_find: every record with one key
 * Takes:   s, a store; key, a key from store_key; visit, called once per
 *          matching record, in the order they were inserted, or NULL to only
 *          count them; ctx, passed to visit unchanged
 * Returns: how many records have that key; 0 for none, or if s is NULL
 */
int store_find(const store_t *s, int key, store_visit_t visit, void *ctx);

/*
 * store_walk: every record, in key order (in-order walk)
 * Takes:   s, a store; visit, called once per record; ctx, passed to visit
 * Returns: nothing. Does nothing if s or visit is NULL.
 */
void store_walk(const store_t *s, store_visit_t visit, void *ctx);

/*
 * store_count: how many records the store holds
 * Returns: the number of records, 0 for NULL
 */
int store_count(const store_t *s);

/*
 * store_height: the height of the tree
 * Returns: -1 until the tree replaces the array in week 8 ("not a tree yet")
 */
int store_height(const store_t *s);

#endif /* STORE_H */
