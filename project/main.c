/*
 * main.c: ECE 361 project 18, Greenhouse vent
 *
 * Usage: ./vent scenarios/NAME.txt            one trace line per tick
 *        ./vent scenarios/NAME.txt --report   only the report (week 9)
 *
 * Replays a scenario file, as hw06/main.c does: for every tick from 0 to the
 * last one it sets the "world" from the line in force, reads the sensor and
 * the switch through the registers, runs the state machine, writes the LEDs
 * and the RGB LED from the state, stores a record, and prints one trace line.
 * The history stack (week 7) and the report (week 9) are not here yet.
 */

#include <stdio.h>          /* fopen, fclose, printf, fprintf */
#include <string.h>         /* strcmp */
#include "iom361_r4.h"      /* iom361_initialize, readReg, writeReg, build_rgb_reg, _iom361_set_... */
#include "sensor.h"         /* sensor_read, reading_t */
#include "scenario.h"       /* scenario_load, scenario_at, scenario_last_tick, scenario_time */
#include "fsm.h"            /* fsm_tick, fsm_leds, fsm_rgb, status_pack, state_name */
#include "store.h"          /* store_create, store_insert, store_destroy, record_t */

#define NUM_SWITCHES 1      /* WIND_ALARM */
#define NUM_LEDS     2      /* MOTOR_OPEN, MOTOR_CLOSE */

static scenario_t sc;       /* the scenario lines; static: 1000 lines are too big for the stack */

int main(int argc, char *argv[]) {

    int report = argc == 3 && strcmp(argv[2], "--report") == 0;    /* 1. the arguments */
    if (argc != 2 && !report) {
        fprintf(stderr, "usage: ./vent scenario.txt [--report]\n");
        return 1;
    }
    if (report) {                                               /*    the report arrives in week 9 */
        fprintf(stderr, "vent: --report is not written yet (week 9)\n");
        return 1;
    }

    FILE *in = fopen(argv[1], "r");                             /* 2. open and load the scenario */
    if (in == NULL) {
        fprintf(stderr, "cannot open %s\n", argv[1]);
        return 1;
    }
    int wrong = scenario_load(&sc, in);                         /*    0, or the first wrong line's number */
    fclose(in);
    if (wrong != 0) {                                           /*    slide 31: report the line, stop */
        fprintf(stderr, "%s: line %d is wrong\n", argv[1], wrong);
        return 1;
    }

    int rc;                                                     /* set by every iom361 call */
    uint32_t *base = iom361_initialize(NUM_SWITCHES, NUM_LEDS, &rc);   /* 3. the hardware */
    if (base == NULL) {
        fprintf(stderr, "iom361_initialize failed: %d\n", rc);
        return 1;
    }
    iom361_set_display(false);                                  /*    the LED writes would print between trace lines */

    store_t *store = store_create();                            /*    every record of the run */
    fsm_timers_t timers;                                        /*    the event code's memory (SPEC.md) */
    fsm_timers_init(&timers, sc.start);
    state_t state = CLOSED;                                     /*    the initial state */
    int store_full = 0;                                         /*    warn only once */
    int last = scenario_last_tick(&sc);                         /* 4. -1 with no data lines: no ticks */

    for (int tick = 0; tick <= last; tick++) {
        scenario_line_t line;                                   /* a. the world, from the line in force */
        scenario_at(&sc, tick, &line);
        _iom361_set_sensor1(line.temp_c, line.rh_pct);          /*    only main sets the world */
        _iom361_set_switches(line.switches);

        time_t now = sc.start + tick;                           /* b. the clock: one second per tick */

        reading_t reading;                                      /* c. the sensor, through the registers */
        rc = sensor_read(base, now, &reading);
        if (rc != 0) {                                          /*    a failed read stops the run */
            fprintf(stderr, "tick %d: sensor_read failed: %d\n", tick, rc);
            store_destroy(store);
            return 1;
        }
        uint32_t switches = iom361_readReg(base, SWITCHES_REG, &rc);   /* d. the switch, through the register */

        action_t action;                                        /* e. the state machine: event, step, timers */
        state = fsm_tick(&timers, state, &reading, switches, NULL, &action);

        uint32_t leds = fsm_leds(state);                        /* f. the outputs, from the state */
        iom361_writeReg(base, LEDS_REG, leds, &rc);             /*    both LEDs belong to the vent: a plain write */
        rgb_t color = fsm_rgb[state];
        iom361_writeReg(base, RGB_LED_REG, build_rgb_reg(1, color.red, color.green, color.blue), &rc);

        uint32_t status = status_pack(state, leds & LED_MOTOR_OPEN, leds & LED_MOTOR_CLOSE,
                                      switches & 0x1u);         /* g. the status word: state, motors, wind */
        record_t record = { .reading = reading, .state = state, .status = status };
        if (store_insert(store, &record) != 0 && !store_full) { /* h. the store: full after STORE_MAX ticks */
            fprintf(stderr, "tick %d: the store is full; later records are not kept\n", tick);
            store_full = 1;
        }

        char when[32];                                          /* i. one trace line, the time in UTC */
        scenario_time(now, when, sizeof when);
        printf("%s state=%s temp=%.1f rh=%.1f leds=0x%x status=0x%08x\n",
               when, state_name(state), reading.temp_c, reading.rh_pct, leds, status);
    }

    store_destroy(store);                                       /* 5. done */
    return 0;
}
