#ifndef tally_H
#define tally_H

#include <stdio.h>      /* FILE, used by tally_print */

#define TALLY_MAX    4    /* tallies that can exist at the same time */
#define TALLY_NAMES  16    /* different names one tally can hold */
#define TALLY_LEN    15    /* longest name, without the '\0' */

typedef struct tally tally_t;      /* opaque */

/*
 * tally_create: a new, empty tally
 * Takes:   nothing.
 * Returns: a pointer to an empty tally, or NULL if TALLY_MAX tallies are
 *          already in use.
 * Assumes: nothing. The tally comes from a fixed pool inside tally.c (no
 *          malloc), so give it back with tally_destroy when you are done.
 */
tally_t *tally_create(void);

/*
 * tally_destroy: gives a tally back so that tally_create can hand it out again
 * Takes:   t, a tally from tally_create, or NULL.
 * Returns: nothing. NULL does nothing.
 * Assumes: t is not used again after this call, and is not destroyed twice.
 */
void tally_destroy(tally_t *t);

/*
 * tally_add: counts one more occurrence of name
 * Takes:   t, a tally from tally_create; name, a '\0'-terminated string.
 * Returns: name's new count (1 the first time). Returns -1 and changes
 *          nothing if t or name is NULL, name is empty or longer than
 *          TALLY_LEN, or name is new and the tally already holds
 *          TALLY_NAMES names.
 * Assumes: the tally copies name, so the caller may reuse or change its
 *          string afterwards.
 */
int tally_add(tally_t *t, const char *name);

/*
 * tally_count: how often name was added
 * Takes:   t, a tally from tally_create; name, a '\0'-terminated string.
 * Returns: the count, 0 if name was never added, and 0 if t or name is NULL.
 * Assumes: nothing; the tally is not changed.
 */
int tally_count(const tally_t *t, const char *name);

/*
 * tally_size: how many different names the tally holds
 * Takes:   t, a tally from tally_create.
 * Returns: the number of different names (0 to TALLY_NAMES), 0 for NULL.
 * Assumes: nothing; the tally is not changed.
 */
int tally_size(const tally_t *t);

/*
 * tally_print: prints the tally, one "name count" line per name
 * Takes:   t, a tally from tally_create; out, an open stream such as stdout.
 * Returns: nothing. Prints nothing if t or out is NULL.
 * Assumes: out is open for writing. The lines come out in alphabetical
 *          (strcmp) order, so uppercase names come before lowercase ones.
 */
void tally_print(const tally_t *t, FILE *out);

#endif // tally_H
