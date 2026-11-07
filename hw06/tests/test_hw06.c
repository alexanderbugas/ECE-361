/*
 * test_hw06.c: unit tests for HW6 (sensor, panel, scenario)
 *
 * Prints PASS or FAIL for each test and a summary. Exits with 1 if any test
 * fails, so that make test fails.
 *
 * Tests the handout requires, at least:
 * -a reading of 91.0 C reads back as 91.0; a failed read changes nothing;
 * -pressed and released for a press, a release, and a held switch;
 *  panel_led and panel_toggle keep the other LEDs;
 * -a load with comments and blank lines; values holding between lines;
 *  a scenario without a start line, with a missing field, and with ticks
 *  that do not increase.
 *
 * Each module's tests are added here as the module is written.
 */

#include <stdio.h>
#include "iom361_r4.h"
#include "sensor.h"
#include <string.h>
#include "panel.h"
#include "scenario.h"

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

/* ---- sensor (Part 1) ---- */

/*
 * A reading of 91.0 C reads back as 91.0, with its timestamp
 *
 * Why: the register gives back 90.99998 C, which is below a 91.0 threshold
 * unless sensor_read rounds (the handout's first test).
 */
static void test_sensor_read(uint32_t *base) {
    reading_t r;

    _iom361_set_sensor1(91.0f, 40.0f);
    check(sensor_read(base, 1791273600, &r) == 0, "sensor: read returns 0");
    check(r.temp_c == 91.0f, "sensor: 91.0 C reads back as 91.0");
    check(r.rh_pct == 40.0f, "sensor: 40.0 %RH reads back as 40.0");
    check(r.t == 1791273600, "sensor: the reading has the timestamp now");

    _iom361_set_sensor1(-12.3f, 0.0f);
    sensor_read(base, 0, &r);
    check(r.temp_c == -12.3f, "sensor: -12.3 C reads back as -12.3 (rounding below 0)");
}

/*
 * A failed read changes nothing and returns the iom361 code
 *
 * Why: the handout's second test; a wrong base address is rc 1.
 */
static void test_sensor_fail(void) {
    uint32_t not_base[8] = { 0 };
    reading_t r = { .t = 7, .temp_c = 1.5f, .rh_pct = 2.5f };

    check(sensor_read(not_base, 99, &r) == 1, "sensor: a wrong base returns rc 1");
    check(r.t == 7 && r.temp_c == 1.5f && r.rh_pct == 2.5f, "sensor: a failed read changes nothing");
}

/* ---- panel (Part 2) ---- */

/*
 * pressed and released for a press, a release, and a held switch
 *
 * Why: a held switch is not a new press (slide 22).
 */
static void test_panel_edges(void) {
    check(panel_pressed(0x1, 0x0) == 0x1 && panel_released(0x1, 0x0) == 0x0, "panel: a press is pressed, not released");
    check(panel_pressed(0x0, 0x1) == 0x0 && panel_released(0x0, 0x1) == 0x1, "panel: a release is released, not pressed");
    check(panel_pressed(0x1, 0x1) == 0x0 && panel_released(0x1, 0x1) == 0x0, "panel: a held switch is neither");
    check(panel_pressed(0x6, 0x3) == 0x4 && panel_released(0x6, 0x3) == 0x1, "panel: several switches at once, bit by bit");
}

/*
 * panel_led and panel_toggle keep the other LEDs; bad n and a failed read
 *
 * Why: the handout's test; writing 1u << n alone would turn the others off.
 */
static void test_panel_leds(uint32_t *base) {
    int rc;

    iom361_writeReg(base, LEDS_REG, 0x5u, &rc);         /* LEDs 0 and 2 on */
    check(panel_led(base, 1, 1) == 0 && iom361_readReg(base, LEDS_REG, &rc) == 0x7u,
          "panel: LED 1 on keeps LEDs 0 and 2 (0x5 -> 0x7)");
    check(panel_led(base, 2, 0) == 0 && iom361_readReg(base, LEDS_REG, &rc) == 0x3u,
          "panel: LED 2 off keeps LEDs 0 and 1 (0x7 -> 0x3)");
    check(panel_toggle(base, 0) == 0 && iom361_readReg(base, LEDS_REG, &rc) == 0x2u,
          "panel: toggle LED 0 keeps LED 1 (0x3 -> 0x2)");
    check(panel_toggle(base, 3) == 0 && iom361_readReg(base, LEDS_REG, &rc) == 0xAu,
          "panel: toggle LED 3 keeps LED 1 (0x2 -> 0xA)");

    check(panel_led(base, 32, 1) == -1 && panel_toggle(base, -1) == -1, "panel: n outside 0 to 31 is -1");
    check(iom361_readReg(base, LEDS_REG, &rc) == 0xAu, "panel: a bad n changes nothing");

    uint32_t not_base[8] = { 0 };
    check(panel_led(not_base, 1, 1) == 1 && panel_toggle(not_base, 1) == 1, "panel: a wrong base returns rc 1");
    check(not_base[1] == 0 && iom361_readReg(base, LEDS_REG, &rc) == 0xAu, "panel: after a failed read nothing is written");
}

/* ---- scenario (Part 3) ---- */

static scenario_t sc;       /* static: 1000 lines is too big for the stack */

/* loads a scenario from text, through a temporary file (H2: an open FILE *) */
static int load(const char *text) {
    FILE *f = tmpfile();                            /* a file that is deleted when closed */
    if (f == NULL) {
        return -100;                                /* no temporary file: the test fails */
    }
    fputs(text, f);
    rewind(f);                                      /* back to the start, to read what was written */
    int result = scenario_load(&sc, f);
    fclose(f);
    return result;
}

/*
 * A load with comments and blank lines
 *
 * Why: the handout's test; comments may also follow the four fields.
 */
static void test_scenario_comments(void) {
    int result = load("# a scenario\n"
                      "\n"
                      "start 1791273600   # 2026-10-06 08:00:00 UTC\n"
                      "# tick temp rh switches\n"
                      "0  21.5 40 0x0\n"
                      "\n"
                      "2  21.5 40 0x1     # switch 0 pressed\n"
                      "5  22.0 45 3\n");                        /* hex without 0x */

    check(result == 0, "scenario: a load with comments and blank lines returns 0");
    check(sc.count == 3 && sc.start == 1791273600, "scenario: 3 lines and the start time are read");
    check(sc.lines[2].switches == 0x3u && sc.lines[2].temp_c == 22.0f, "scenario: hex switches without 0x");
}

/*
 * Values hold between lines; nothing is in force before the first line
 *
 * Why: the handout's test; the replay asks for every tick, not only the
 * ticks in the file.
 */
static void test_scenario_hold(void) {
    scenario_line_t line;

    load("start 0\n0 21.5 40 0x0\n2 22.0 41 0x1\n4 23.0 42 0x3\n");
    check(scenario_at(&sc, 1, &line) == 0 && line.tick == 0, "scenario: tick 1 holds the tick-0 line");
    check(scenario_at(&sc, 3, &line) == 0 && line.tick == 2 && line.switches == 0x1u,
          "scenario: tick 3 holds the tick-2 line");
    check(scenario_at(&sc, 4, &line) == 0 && line.tick == 4, "scenario: tick 4 is the tick-4 line");
    check(scenario_at(&sc, 9, &line) == 0 && line.tick == 4, "scenario: after the last line, it holds");
    check(scenario_at(&sc, -1, &line) == -1, "scenario: before tick 0 nothing is in force");
    check(scenario_last_tick(&sc) == 4, "scenario: the last tick is 4");
}

/*
 * Wrong files: the number of the first wrong line, counting every line
 *
 * Why: the handout's three cases, and the settled ones (an extra word, a
 * first tick that is not 0, a file with no start line at all).
 */
static void test_scenario_wrong(void) {
    check(load("0 21.5 40 0x0\n") == 1, "scenario: no start line: line 1");
    check(load("start 0\n0 21 40 0\n5 21 40\n") == 3, "scenario: a missing field: line 3");
    check(load("start 0\n0 21 40 0\n5 21 40 0x1\n5 22 40 0x1\n") == 4, "scenario: ticks that do not increase: line 4");
    check(load("start 0\n0 21 40 0\n5 21 40 0x1 POWER\n") == 3, "scenario: an extra word: line 3");
    check(load("# c\nstart 0\n3 21 40 0\n") == 3, "scenario: a first tick that is not 0: line 3");
    check(load("") == 1 && load("# only\n\n# comments\n") == 1, "scenario: an empty file, or only comments: 1");
}

/*
 * The clock: the sheet's four cases, printed in UTC
 *
 * Why: the project sheet's clock tests (the clock lives in scenario.c).
 */
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

int main(void) {
    int rc;
    uint32_t *base = iom361_initialize(4, 4, &rc);     /* as replay does */

    if (base == NULL) {
        printf("FAIL  iom361_initialize: rc %d\n", rc);
        return 1;
    }
    iom361_set_display(false);      /* the LED writes would print between the test lines */

    test_sensor_read(base);
    test_sensor_fail();
    test_panel_edges();
    test_panel_leds(base);
    test_scenario_comments();
    test_scenario_hold();
    test_scenario_wrong();
    test_clock();

    printf("\n%d passed, %d failed\n", passed, failed);
    return failed > 0 ? 1 : 0;
}
