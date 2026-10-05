#include <stdio.h>
#include "bits.h"
#include "status.h"

static int fails = 0;
#define CHECK(cond) do { \
    if (cond) printf("PASS %s\n", #cond); \
    else { printf("FAIL %s (line %d)\n", #cond, __LINE__); fails++; } \
} while (0)

int main(void) {
    print_binary(0x2C, 8);                                      /* expect 0010 1100 */
    CHECK(get_field(0x1631, 0, 1) == 1);                        /* width 1: bit 0 */
    CHECK(get_field(0xDEADBEEF, 0, 32) == 0xDEADBEEF);          /* width 32: whole word */
    CHECK(get_field(0x80000000, 31, 1) == 1);                   /* pos 31: top bit */
    CHECK(get_field(0xFF, 30, 3) == 0);                         /* out of range: pos + width > 32 */
    CHECK(set_field(0, 0, 1, 1) == 1);                          /* width 1: set bit 0 */
    CHECK(set_field(0xDEADBEEF, 0, 32, 0xCAFEBABE) == 0xCAFEBABE); /* width 32: replace whole word */
    CHECK(set_field(0, 31, 1, 1) == 0x80000000);                /* pos 31: set top bit */
    CHECK(set_field(0, 4, 3, 0xFF) == 0x70);                    /* value too wide: only 3 bits kept */
    CHECK(set_field(0x1234, 30, 3, 1) == 0);                    /* out of range: returns 0 */
    CHECK(sign_extend(0x7FFFFFFF, 32) == 2147483647);           /* width 32: largest positive */
    CHECK(sign_extend(0x80, 8) == -128);                        /* most negative 8-bit */
    CHECK(sign_extend(1, 1) == -1);                             /* width 1: lone sign bit is -1 */
    CHECK(sign_extend(0xF8, 8) == -8);                          /* spec example */
    CHECK(sign_extend(0x80000000, 32) == -2147483648);          /* most negative 32-bit */
    CHECK(status_unpack(0x1631).heat == 1);                     /* call result used directly */

    /* spec example: 22 C, AUTO, heater on, everything else off */
    status_t s = status_unpack(0x1631);
    CHECK(s.setpoint == 22);                                    /* 0x16 = 22 */
    CHECK(s.mode == MODE_AUTO);                                 /* bits 6-4 = 3 */
    CHECK(s.heat == 1);                                         /* bit 0 set */
    CHECK(s.cool == 0);                                         /* bit 1 clear */
    CHECK(s.fan == 0);                                          /* bit 2 clear */
    CHECK(s.fault == 0);                                        /* bit 3 clear */
    CHECK(s.reserved == 0);                                     /* bit 7 clear */
    CHECK(s.mode_invalid == 0);                                 /* mode 3 is valid */


    /* lecture example: negative set point with a fault */
    s = status_unpack(0xF819);
    CHECK(s.setpoint == -8);                                    /* 0xF8 sign-extended */
    CHECK(s.mode == MODE_HEAT);                                 /* bits 6-4 = 1 */
    CHECK(s.heat == 1);                                         /* bit 0 set */
    CHECK(s.fault == 1);                                        /* bit 3 set */

    /* invalid mode: all mode bits set */
    s = status_unpack(0x0070);
    CHECK(s.setpoint == 0);                                     /* upper byte empty */
    CHECK(s.mode == 7);                                         /* raw value kept */
    CHECK(s.heat == 0);                                         /* bit 0 clear */

    CHECK(s.mode_invalid == 1);                                 /* mode 7 flagged */

    printf("%d failed\n", fails);
    return fails != 0;
}
