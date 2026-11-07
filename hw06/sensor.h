#ifndef SENSOR_H
#define SENSOR_H

#include <stdint.h>
#include <time.h>

typedef struct {
    time_t t;           /* seconds since 1970-01-01 00:00:00 UTC */
    float temp_c;       /* degrees C */
    float rh_pct;       /* %RH */
} reading_t;

int sensor_read(uint32_t *base, time_t now, reading_t *r); /*  reads TEMP_REG and HUMID_REG, keeps bits 19..0 of each, 
                                                             converts them with the formulas in iom361_r4.h,
                                                            rounds to 0.1, and stores them with the timestamp now in *r.
                                                            Returns 0, or the iom361 return code of a failed read; then it changes nothing.
                                                            r must not be NULL. */


                                                            
#endif // SENSOR_H