#include <stdio.h>
#include <string.h>
#include "tally.h"

/* counts how often each name occurs
Caller uses it only through its header
only tally.c knows what is inside tally.h */


////////////////////////
/*FUNCTION DEFINITIONS*/
////////////////////////

/* entry name and times appeared */
struct entry {

    char name[TALLY_LEN + 1];   /* the name entry with room for the NULL character */
    int count;      /* how many entries of that name there are*/
};

/* one tally: its entries, how many are filled, and whether it's handed out */
struct tally {

    struct entry entries[TALLY_NAMES]; /* an arry of entries, each a name and count */
    int num_names;
    int in_use;
};

static struct tally datapool[TALLY_MAX];

/* a new, empty tally; NULL if TALLY_MAX tallies are already in use */
tally_t *tally_create(void){

    for (int i = 0;  i < TALLY_MAX; i++){ /* how many tallies in the datapool*/

        if( datapool[i].in_use == 0 ){ /* search parameter: "is this slot free?" */

            datapool[i].in_use = 1;
            datapool[i].num_names = 0;

            return &datapool[i];
        }

    }
    return NULL;
}

/* gives the tally back so that tally_create can hand it out again; NULL does nothing */
void tally_destroy(tally_t *t){
    
    if (t == NULL){     /* NULL does nothing */
        return;
    }

    t->in_use = 0;

}

/* counts one more occurrence of name and returns its new count; returns −1 and changes 
nothing if t or name is NULL, name is empty or longer than TALLY_LEN, or name is new and 
the tally is full */
int tally_add(tally_t *t, const char *name){

    if (t == NULL || name == NULL || name[0] == '\0' || strlen(name) > TALLY_LEN){
        return -1;
    }

    int i;      /* declared here so it still holds the insert spot after the loop */
    for (i = 0; i < t->num_names; i++){     /* search the filled entries */

        int cmp = strcmp(name, t->entries[i].name);

        if (cmp == 0){      /* already here: count it once more */
            t->entries[i].count++;          /* one more occurrence of this name */
            return t->entries[i].count;     /* the new count; nothing moves, so still sorted */
        }

        if (cmp < 0){       /* name comes before entries[i]: insert at i */
            break;
        }
    }
    /* i is the insert spot: where the loop broke, or num_names if it never did */

    if (t->num_names == TALLY_NAMES){
        return -1;
    }

    for (int j = t->num_names; j > i; j--){
        t->entries[j] = t->entries[j-1];
    }

    /* only runs if new entry for tally*/
    strcpy(t->entries[i].name, name);   /* length already checked */
    t->entries[i].count =1;             /* first occurence */
    t->num_names++;                     /* one more different name */
    return 1;                           /* new count */
    
}

/* how often name was added: 0 if never, or for NULL */
int tally_count(const tally_t *t, const char *name){

    if (t == NULL || name == NULL){     /* 0 for NULL */
        return 0;
    }

    for (int i = 0; i < t->num_names; i++){     /* only the filled entries */
        if (strcmp(name, t->entries[i].name) == 0){     /* 0 means a match */
            return t->entries[i].count;
        }
    }

    return 0;
}

/*how many different names the tally holds: 0 for NULL */
int tally_size(const tally_t *t){

    if (t == NULL){
        return 0;
    }

    return t->num_names;
}

/*one line per name, name count, in alphabetical (strcmp) order */
void tally_print(const tally_t *t, FILE *out){
    if(t == NULL || out == NULL){
        return;
    }

    for (int i = 0; i < t->num_names; i++){

        fprintf(out, "%s %d\n", t->entries[i].name, t->entries[i].count);

    }

}