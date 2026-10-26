#include <stdio.h>
#include <string.h>
#include "tally.h"


/*
Reads standard input line by line with fgets, removes the newline, skips blank lines
and lines that start with #, adds every other line to one tally, then  prints the tally and one last line, names:

The last line is:
names: N, lines skipped: S, where S counts the lines tally_add refused.
*/

int main(void) {

    /*create one tally, if its null print an error and return 1*/
    tally_t *t = tally_create();
    if (t == NULL){
        fprintf(stderr, "events: could not create a tally\n");
        return 1;
    }

    /*a char buffer for one line, skipped = 0*/
    char input[128];        /* much longer than TALLY_LEN, so a long name arrives whole */
    int skipped = 0;        /* lines tally_add refused */

    /*while fgets reads a line into the buffer:
        remove the newline
        if the line is empty -> skip it, next line
        if the first char is '#' -> skip it, next line
        if tally_add returns -1 -> skipped goes up by 1
    */

    while (fgets(input, sizeof input, stdin) != NULL){

        size_t len = strlen(input);

        if(len > 0 && input[len - 1] == '\n'){ /* is the last character a newline? */
            input[len - 1] = '\0';             /* overwrite it: the string ends one earlier */
        }

        if (input[0] == '\0'){      /* blank line: skip it, not counted */
            continue;
        }

        if (input[0] == '#'){       /* comment line: skip it, not counted */
            continue;
        }

        if (tally_add(t, input) == -1){     /* refused, e.g. longer than TALLY_LEN */
            skipped++;
        }
    }

    /*print the tally*/
    tally_print(t, stdout);

    /*print "names: N, lines skipped: S" (N = tally size)*/
    printf("names: %d, lines skipped: %d\n", tally_size(t), skipped);

    /*destroy the tally*/
    tally_destroy(t);

    return 0;
}
