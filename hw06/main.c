/*
 * main.c: HW6, the device loop
 *
 * Usage: ./replay scenarios/NAME.txt
 *
 * Replays a scenario file: for every tick from 0 to the last one it sets the
 * "world" (sensor and switches) from the line in force, reads them back
 * through the registers, toggles LED k for every switch k just pressed, and
 * prints one trace line. The steps are numbered as in the pseudocode at the
 * bottom of this file.
 */

#include <stdio.h>          /* fopen, fclose, printf, fprintf */
#include "iom361_r4.h"      /* iom361_initialize, readReg, set_display, _iom361_set_... */
#include "sensor.h"         /* sensor_read, reading_t */
#include "panel.h"          /* panel_pressed, panel_toggle */
#include "scenario.h"       /* scenario_load, scenario_at, scenario_last_tick, scenario_time */

static scenario_t sc;       /* the scenario lines, loaded from the file; static: 1000 lines
                               are too big for the stack */

int main(int argc, char *argv[]) {

    if (argc != 2) {                                            /* 1. one argument: the scenario file's name */
        fprintf(stderr, "usage: ./replay scenario.txt\n");
        return 1;
    }

    FILE *in = fopen(argv[1], "r");                             /* 2. open the scenario (H2: main opens it) */
    if (in == NULL) {                                           /*    it does not exist, or cannot be read */
        fprintf(stderr, "cannot open %s\n", argv[1]);
        return 1;
    }
    int wrong = scenario_load(&sc, in);                         /*    0, or the first wrong line's number */
    fclose(in);                                                 /*    the lines are in sc now */
    if (wrong != 0) {                                           /*    slide 31: report the line, stop */
        fprintf(stderr, "%s: line %d is wrong\n", argv[1], wrong);
        return 1;
    }

    int rc;                                                     /* set by every iom361 call */
    uint32_t *base = iom361_initialize(4, 4, &rc);              /* 3. the hardware: 4 switches, 4 LEDs */
    if (base == NULL) {
        fprintf(stderr, "iom361_initialize failed: %d\n", rc);
        return 1;
    }
    iom361_set_display(false);                                  /*    the LED writes would print between trace lines */

    uint32_t before = 0;                                        /* 4. the switches on the tick before: none yet */
    int last = scenario_last_tick(&sc);                         /*    -1 with no data lines: the loop runs 0 times */

    for (int tick = 0; tick <= last; tick++) {
        scenario_line_t line;                                   /* a. the world, from the line in force */
        scenario_at(&sc, tick, &line);                          /*    H3: copied into line */
        _iom361_set_sensor1(line.temp_c, line.rh_pct);          /*    only main sets the world */
        _iom361_set_switches(line.switches);

        time_t now = sc.start + tick;                           /* b. the clock: one second per tick */

        reading_t reading;                                      /* c. the sensor, through the registers */
        rc = sensor_read(base, now, &reading);
        if (rc != 0) {                                          /*    H4: a failed read stops the run */
            fprintf(stderr, "tick %d: sensor_read failed: %d\n", tick, rc);
            return 1;
        }

        uint32_t switches = iom361_readReg(base, SWITCHES_REG, &rc);   /* d. the switches, through the register,
                                                                          not line.switches */
        uint32_t pressed = panel_pressed(switches, before);    /*    off before, on now */

        for (int k = 0; k < 4; k++) {                           /* e. LED k toggles for every switch k pressed */
            if ((pressed >> k) & 1) {                           /*    is bit k of pressed 1? */
                panel_toggle(base, k);
            }
        }

        uint32_t leds = iom361_readReg(base, LEDS_REG, &rc);    /* f. the LEDs, read back for the trace */

        char when[32];                                          /* g. one trace line, the time in UTC */
        scenario_time(now, when, sizeof when);
        printf("%s temp=%.1f rh=%.1f switches=0x%x pressed=0x%x leds=0x%x\n",   /* 0x%x, not %#x: %#x */
               when, reading.temp_c, reading.rh_pct, switches, pressed, leds); /* prints 0 as "0" */

        before = switches;                                      /* h. saved last, for the next tick (slide 22) */
    }
    return 0;                                                   /* 5. done */
}

/*
 * PSEUDOCODE (the plan main() was written from)
 *
 * main(argc, argv):
 *
 *   1. the argument
 *     if argc is not 2                      one argument: the scenario file's name
 *         print "usage: ./replay scenario.txt" on stderr
 *         return 1
 *
 *   2. open and load the scenario           [H2: main opens the file]
 *     in = fopen(argv[1], "r")
 *     if in is NULL                         the file does not exist, or cannot be read
 *         print "cannot open <name>" on stderr
 *         return 1
 *     wrong = scenario_load(&sc, in)
 *     fclose(in)                            the lines are in sc now; the file is not needed
 *     if wrong is not 0                     [settled: slide 31, "report line, stop"]
 *         print "<name>: line <wrong> is wrong" on stderr
 *         return 1
 *
 *   3. the hardware
 *     base = iom361_initialize(4, 4, &rc)   4 switches, 4 LEDs (the handout)
 *     if base is NULL
 *         print "iom361_initialize failed: <rc>" on stderr
 *         return 1
 *     iom361_set_display(false)             the LED writes would print between trace lines
 *
 *   4. the loop
 *     before = 0                            the switches on the tick before: none yet
 *     last = scenario_last_tick(&sc)        -1 if there are no data lines: the loop runs 0 times
 *
 *     for tick from 0 to last (inclusive):
 *
 *         a. set the world from the line in force     [only main does this]
 *             scenario_at(&sc, tick, &line)            H3: a copy into line
 *             _iom361_set_sensor1(line.temp_c, line.rh_pct)
 *             _iom361_set_switches(line.switches)
 *
 *         b. the clock
 *             now = sc.start + tick                    one second per tick
 *
 *         c. read the sensor through the registers
 *             rc = sensor_read(base, now, &reading)
 *             if rc is not 0                           [H4: stop]
 *                 print "tick <tick>: sensor_read failed: <rc>" on stderr
 *                 return 1
 *
 *         d. read the switches through the register    not line.switches: the device
 *             switches = iom361_readReg(base, SWITCHES_REG, &rc)   reads what the world set
 *             pressed = panel_pressed(switches, before)
 *
 *         e. toggle LED k for every switch k just pressed
 *             for k from 0 to 3:                       4 switches, 4 LEDs
 *                 if bit k of pressed is 1             (pressed >> k) & 1
 *                     panel_toggle(base, k)
 *
 *         f. read the LEDs back, for the trace
 *             leds = iom361_readReg(base, LEDS_REG, &rc)
 *
 *         g. print one trace line
 *             scenario_time(now, when, sizeof when)    char when[32]; UTC
 *             printf("%s temp=%.1f rh=%.1f switches=0x%x pressed=0x%x leds=0x%x\n",
 *                    when, reading.temp_c, reading.rh_pct, switches, pressed, leds)
 *                                                      0x%x, not %#x: %#x prints 0 as "0"
 *
 *         h. remember the switches for the next tick   [slide 22: saved last]
 *             before = switches
 *
 *   5. the end
 *     return 0
 */
