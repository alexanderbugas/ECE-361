#ifndef SCENARIO_H
#define SCENARIO_H

#include <stdio.h>
#include <stdint.h>
#include <time.h>

#define SCENARIO_MAX 1000

typedef struct { int tick; float temp_c; float rh_pct; uint32_t switches; } scenario_line_t;

typedef struct {
    time_t start;                     /* from the "start" line */
    scenario_line_t lines[SCENARIO_MAX];   /* SCENARIO_MAX = 1000, no malloc */
    int count;
} scenario_t;

int scenario_load(scenario_t *sc, FILE *in);          /* 0, or the first wrong line's number (H2: an open FILE *) */
int scenario_at(const scenario_t *sc, int tick, scenario_line_t *out);   /* 0, or -1 if no line in force (H3: a copy) */
int scenario_last_tick(const scenario_t *sc);         /* main replays 0 to this */
void scenario_time(time_t t, char *buf, size_t size); /* the clock: "2026-10-06 08:00:46", UTC */


#endif // SCENARIO_H