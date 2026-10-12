/*
 * main.c: ECE 361 project 18, Greenhouse vent
 *
 * For now: sets up the iom361 hardware model with the vent's 1 switch
 * (WIND_ALARM) and 2 LEDs (MOTOR_OPEN, MOTOR_CLOSE), and prints the name of
 * the device. Returns 1 if the setup fails, 0 otherwise.
 */
#include <stdio.h>
#include "iom361_r4.h"

#define NUM_SWITCHES 1
#define NUM_LEDS     2

int main(void) {
    int rc;
    uint32_t *base = iom361_initialize(NUM_SWITCHES, NUM_LEDS, &rc);

    if (base == NULL) {
        fprintf(stderr, "iom361_initialize failed: %d\n", rc);
        return 1;
    }
    printf("Greenhouse vent\n");
    return 0;
}
