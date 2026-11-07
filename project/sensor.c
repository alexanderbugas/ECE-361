#include "sensor.h"
#include "iom361_r4.h"

#define RAW_MASK 0xFFFFFu   /* bits 19..0: the 20-bit raw value */

/* Both reads first, then *r: a failed read returns before anything is
   written, as hw03/ptr.c's minmax does. The rounding is integers only
   (week 6, slides 8 to 10). */
int sensor_read(uint32_t *base, time_t now, reading_t *r) {
    int return_code;                                                            /* set by every iom361 call */

    uint32_t raw_temp = iom361_readReg(base, TEMP_REG, &return_code) & RAW_MASK; /* read; keep bits 19..0 (the upper bits
                                                                                    could be status bits on a real board) */
    if (return_code != 0) {                                                     /* the read failed: */
        return return_code;                                                     /* return its code, *r untouched */
    }
    uint32_t raw_humid = iom361_readReg(base, HUMID_REG, &return_code) & RAW_MASK; /* the same for humidity */
    if (return_code != 0) {                                                     /* only when both reads worked */
        return return_code;                                                     /* does the function go on */
    }

    int32_t temp_tenths = (int32_t)((raw_temp * 2000u + (1u << 19)) >> 20) - 500; /* C = raw / 2^20 * 200 - 50, in tenths:
                                                                                    * 2000 (200 C x 10 tenths) before dividing,
                                                                                    + 2^19 (half of 2^20) so >> 20 rounds,
                                                                                    >> 20 divides by 2^20, - 500 is the -50 C
                                                                                    offset; signed for cold readings.
                                                                                    raw 0xB47AE -> 910 (909 without the half) */
    int32_t humid_tenths = (int32_t)((raw_humid * 1000u + (1u << 19)) >> 20);   /* %RH = raw / 2^20 * 100, in tenths: the same
                                                                                    steps with 1000, no offset (never below 0) */

    r->t = now;                                                                 /* both reads worked: now *r is written */
    r->temp_c = temp_tenths / 10.0f;                                            /* tenths back to a float: 910 -> 91.0 */
    r->rh_pct = humid_tenths / 10.0f;                                           /* (the f makes it float division) */
    return 0;                                                                   /* success */
}
