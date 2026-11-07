#include <math.h>       /* floorf */
#include <stddef.h>     /* NULL */
#include "store.h"

/* keeps every record, sorted by key; caller uses it only through store.h.
   For now a sorted array in a fixed pool, as hw04/tally.c (no malloc). */

/* one store: its records in key order, how many there are, and whether it
   is handed out */
struct store {
    record_t records[STORE_MAX];
    int count;
    int in_use;
};

static struct store pool[STORE_POOL];

/* the 0.5 C step of a temperature; floorf rounds down below 0 as well */
int store_key(float temp_c) {
    return (int)floorf(temp_c * 2.0f);
}

/* compare the keys, not the temperatures: 21.2 and 21.4 are equal */
int record_compare(const record_t *a, const record_t *b) {
    int ka = store_key(a->reading.temp_c);
    int kb = store_key(b->reading.temp_c);

    return (ka > kb) - (ka < kb);       /* -1, 0 or 1 */
}

/* a free store from the pool, emptied; NULL if all are in use */
store_t *store_create(void) {
    for (int i = 0; i < STORE_POOL; i++) {
        if (pool[i].in_use == 0) {          /* is this slot free? */
            pool[i].in_use = 1;
            pool[i].count = 0;
            return &pool[i];
        }
    }
    return NULL;
}

/* give the store back to the pool; NULL does nothing */
void store_destroy(store_t *s) {
    if (s == NULL) {
        return;
    }
    s->in_use = 0;
}

/* insert after every record with a lower or equal key, as tally_add does,
   so records with the same key stay in the order they arrived */
int store_insert(store_t *s, const record_t *r) {
    if (s == NULL || r == NULL || s->count == STORE_MAX) {
        return -1;
    }

    int i;      /* declared here so it still holds the insert spot after the loop */
    for (i = 0; i < s->count; i++) {
        if (record_compare(r, &s->records[i]) < 0) {    /* r goes before records[i] */
            break;
        }
    }
    /* i is the insert spot: where the loop broke, or count if it never did */

    for (int j = s->count; j > i; j--) {    /* move the rest up one */
        s->records[j] = s->records[j - 1];
    }
    s->records[i] = *r;
    s->count++;
    return 0;
}

/* the records with this key sit next to each other, in insertion order */
int store_find(const store_t *s, int key, store_visit_t visit, void *ctx) {
    if (s == NULL) {
        return 0;
    }

    int found = 0;
    for (int i = 0; i < s->count; i++) {
        int k = store_key(s->records[i].reading.temp_c);

        if (k > key) {          /* sorted: nothing further can match */
            break;
        }
        if (k == key) {
            if (visit != NULL) {
                visit(&s->records[i], ctx);
            }
            found++;
        }
    }
    return found;
}

/* the array is already in key order, so the walk is one pass */
void store_walk(const store_t *s, store_visit_t visit, void *ctx) {
    if (s == NULL || visit == NULL) {
        return;
    }
    for (int i = 0; i < s->count; i++) {
        visit(&s->records[i], ctx);
    }
}

/* number of records; 0 for NULL */
int store_count(const store_t *s) {
    if (s == NULL) {
        return 0;
    }
    return s->count;
}

/* not a tree yet (week 8) */
int store_height(const store_t *s) {
    (void)s;
    return -1;
}
