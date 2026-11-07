#include "panel.h"
#include "iom361_r4.h"      /* iom361_readReg, iom361_writeReg, LEDS_REG */

/* off before, on now (week 6, slide 22) */
uint32_t panel_pressed(uint32_t now, uint32_t before) {
    return now & ~before;                           /* ~before has a 1 where a switch was off; & now keeps
                                                       the ones that are on now: 0x1 now, 0x0 before -> 0x1 */
}

/* on before, off now */
uint32_t panel_released(uint32_t now, uint32_t before) {
    return before & ~now;                           /* the same with the roles swapped: on before, off now */
}

/* read, change one bit, write the whole register back (slides 19 and 20) */
int panel_led(uint32_t *base, int n, int on) {
    if (n < 0 || n > 31) {                          /* the register has 32 bits, 0 to 31 */
        return -1;                                  /* a bad n: nothing is read or written */
    }

    int return_code;                                /* set by every iom361 call */
    uint32_t leds = iom361_readReg(base, LEDS_REG, &return_code); /* 1. read every LED at once */
    if (return_code != 0) {                         /* the read failed: leds holds 0xDEADBEEF, and */
        return return_code;                         /* writing it back would light the LEDs, so stop */
    }

    if (on) {
        leds |= 1u << n;                            /* 2. modify: OR sets bit n, the others stay */
    } else {
        leds &= ~(1u << n);                         /*    AND with a 0 at bit n clears it, the others stay */
    }
    iom361_writeReg(base, LEDS_REG, leds, &return_code); /* 3. write the whole register back; writing
                                                            1u << n alone would turn the others off */
    return return_code;                             /* 0, or the code of the failed write */
}

/* the same read-modify-write, with ^ to flip the bit */
int panel_toggle(uint32_t *base, int n) {
    if (n < 0 || n > 31) {                          /* the same check as panel_led */
        return -1;
    }

    int return_code;
    uint32_t leds = iom361_readReg(base, LEDS_REG, &return_code); /* 1. read */
    if (return_code != 0) {                         /* no write after a failed read */
        return return_code;
    }

    leds ^= 1u << n;                                /* 2. XOR flips bit n: 1 -> 0, 0 -> 1 */
    iom361_writeReg(base, LEDS_REG, leds, &return_code); /* 3. write the whole register back */
    return return_code;
}
