/*
 * sensor: reads the greenhouse's temperature and humidity from the AHT20
 * registers of iom361 and converts the raw 20-bit values to degrees C and
 * %RH, rounded to 0.1. Each reading is stamped with the time it was taken.
 * The vent's decisions are made elsewhere; this module only measures.
 * It prints nothing, and it never calls iom361's _iom361_set_... functions.
 *
 * reading_t and sensor_read are the ones from the HW6 handout, Part 1.
 */
#ifndef SENSOR_H
#define SENSOR_H

#include <stdint.h>     /* uint32_t, for the register base */
#include <time.h>       /* time_t */

/* one reading: when it was taken, and the two values rounded to 0.1 */
typedef struct {
    time_t t;           /* seconds since 1970-01-01 00:00:00 UTC */
    float temp_c;       /* degrees C */
    float rh_pct;       /* %RH */
} reading_t;

/*
 * sensor_read: reads TEMP_REG and HUMID_REG, keeps bits 19..0 of each,
 * converts them with the formulas in iom361_r4.h, and rounds to 0.1 with
 * integers only (SPEC.md, "Readings")
 * Takes:   base, the address iom361_initialize returned;
 *          now, the timestamp for this reading;
 *          r, where to store the reading
 * Returns: 0, with the reading in *r; or the iom361 return code of the read
 *          that failed, and then *r is not changed
 * Assumes: r is not NULL
 */
int sensor_read(uint32_t *base, time_t now, reading_t *r);

#endif /* SENSOR_H */
