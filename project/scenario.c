#include "scenario.h"
#include <stdio.h>
#include <string.h>


/* 1 if the line holds only spaces, tabs and the newline (after the comment
   is cut off), 0 if not: hw02/reading.c's is_blank_or_comment, without the '#' */
static int is_blank(const char line[]) {
    int i = 0;
    while (line[i] == ' ' || line[i] == '\t')
        i++;
    return line[i] == '\n' || line[i] == '\0';
}

/* 0, or the first wrong line's number (H2: an open FILE *) */
int scenario_load(scenario_t *sc, FILE *in){

    char buffer[128];                                   /* one line of the file */
    int line_number = 0;                                /* counts every line, comments and blanks too */
    int have_start = 0;                                 /* 1 once the "start" line is read */
    sc->count = 0;

    while (fgets(buffer, sizeof(buffer), in) != NULL) { /* one line at a time, until the end of the file */
        line_number = line_number + 1;

        char *hash = strchr(buffer, '#');               /* pointer to the first '#', or NULL */
        if (hash != NULL) {
            *hash = '\0';                               /* the string now ends at the '#' */
        }
        if (is_blank(buffer)) {                         /* a blank line, or one that was only a comment */
            continue;
        }

        char extra[2];                                  /* one more word, if %1s finds one */

        if (!have_start) {                              /* the first real line must be "start <timestamp>" */
            long long start;                            /* %lld reads a long long; time_t is cast from it */
            if (sscanf(buffer, " start %lld %1s", &start, extra) != 1) {
                return line_number;                     /* no start line, or something after the number */
            }
            sc->start = (time_t)start;
            have_start = 1;
            continue;
        }

        int tick;
        float temp, rh;
        unsigned int sw;                                /* %x writes an unsigned int */
        int n = sscanf(buffer, "%d %f %f %x %1s", &tick, &temp, &rh, &sw, extra);

        if (n != 4) {                                   /* fewer: a field is missing; 5: something extra */
            return line_number;
        }
        if (sc->count == 0 && tick != 0) {              /* replay starts at tick 0 */
            return line_number;
        }
        if (sc->count > 0 && tick <= sc->lines[sc->count - 1].tick) {  /* the ticks must increase */
            return line_number;
        }
        if (sc->count == SCENARIO_MAX) {                /* no room: at most 1000 lines, no malloc */
            return line_number;
        }

        scenario_line_t *line = &sc->lines[sc->count];  /* the next free line */
        line->tick = tick;
        line->temp_c = temp;
        line->rh_pct = rh;
        line->switches = sw;
        sc->count++;
    }

    if (!have_start) {                                  /* an empty file, or only comments */
        return 1;
    }
    return 0;
}

/* 0, or -1 if no line in force (H3: a copy) */
int scenario_at(const scenario_t *sc, int tick, scenario_line_t *out) {
    if (sc->count == 0 || tick < sc->lines[0].tick) {
        return -1; // No lines in force
    }

    int found = 0;                          // index of the line in force; line 0 is, by the check above

    for (int i = 0; i < sc->count; i++) {
        if (sc->lines[i].tick <= tick) {
            found = i;                      // this line has started: the latest one so far
        } else {
            break;                          // the ticks increase, so no later line can be in force
        }
    }

    *out = sc->lines[found];                // Copy the line to out (H3)
    return 0;
}

/* main replays 0 to this */
int scenario_last_tick(const scenario_t *sc){
    if (sc->count == 0) {
        return -1;                          // no lines: nothing to replay
    }
    return sc->lines[sc->count - 1].tick;   // the ticks increase, so the last line has the largest
}

/* the clock: "2026-10-06 08:00:46", UTC */
void scenario_time(time_t t, char *buf, size_t size){
    struct tm *tm_info = gmtime(&t);
    if (tm_info == NULL) {
        buf[0] = '\0'; // Handle error case
        return;
    }
    strftime(buf, size, "%Y-%m-%d %H:%M:%S", tm_info);
}