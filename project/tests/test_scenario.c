/*
 * test_scenario.c: unit tests for scenario.h / scenario.c (from HW6), and
 * the clock, which lives there
 */
#include <stdio.h>
#include <string.h>
#include "check.h"
#include "scenario.h"

static scenario_t sc;       /* static: 1000 lines are too big for the stack */

/* loads a scenario from text, through a temporary file */
static int load(const char *text) {
    FILE *f = tmpfile();                                        /* deleted when closed */
    if (f == NULL) {
        return -100;                                            /* no temporary file: the test fails */
    }
    fputs(text, f);
    rewind(f);                                                  /* back to the start, to read it */
    int result = scenario_load(&sc, f);
    fclose(f);
    return result;
}

/* a load with comments and blank lines; values hold between lines */
static void test_load_and_hold(void) {
    scenario_line_t line;

    check(load("# the sheet's example\n"
               "start 1791273600   # 2026-10-06 08:00:00 UTC\n"
               "# tick temp rh switches\n"
               "0  26.0 70 0x0\n"
               "\n"
               "5  26.0 70 0x1     # WIND_ALARM on\n"
               "60 27.0 70 0x1\n") == 0, "scenario: the sheet's example loads");
    check(sc.count == 3 && sc.start == 1791273600, "scenario: 3 lines and the start time");
    check(scenario_at(&sc, 4, &line) == 0 && line.tick == 0, "scenario: tick 4 holds the tick-0 line");
    check(scenario_at(&sc, 59, &line) == 0 && line.tick == 5 && line.switches == 0x1u,
          "scenario: tick 59 holds the tick-5 line");
    check(scenario_last_tick(&sc) == 60, "scenario: the last tick is 60");
    check(scenario_at(&sc, -1, &line) == -1, "scenario: before tick 0 nothing is in force");
}

/* wrong files: the number of the first wrong line */
static void test_wrong(void) {
    check(load("0 21.5 40 0x0\n") == 1, "scenario: no start line: line 1");
    check(load("start 0\n0 21 40 0\n5 21 40\n") == 3, "scenario: a missing field: line 3");
    check(load("start 0\n0 21 40 0\n5 21 40 0x1\n5 22 40 0x1\n") == 4, "scenario: ticks that do not increase: line 4");
    check(load("start 0\n0 21 40 0\n5 21 40 0x1 POWER\n") == 3, "scenario: an extra word: line 3");
    check(load("start 0\n3 21 40 0\n") == 2, "scenario: a first tick that is not 0: line 2");
    check(load("") == 1, "scenario: an empty file: 1");
}

/* the sheet's clock tests, in UTC */
static void test_clock(void) {
    char text[32];

    scenario_time(1791273600, text, sizeof text);
    check(strcmp(text, "2026-10-06 08:00:00") == 0, "clock: the start time prints as given");
    scenario_time(1791273600 + 59, text, sizeof text);
    check(strcmp(text, "2026-10-06 08:00:59") == 0, "clock: 59 seconds later");
    scenario_time(1791331199 + 1, text, sizeof text);
    check(strcmp(text, "2026-10-07 00:00:00") == 0, "clock: one second after 23:59:59 is the next day");
    scenario_time(1803859199 + 1, text, sizeof text);
    check(strcmp(text, "2027-03-01 00:00:00") == 0, "clock: 28 February 2027 to 1 March");
}

void test_scenario(void) {
    test_load_and_hold();
    test_wrong();
    test_clock();
}
