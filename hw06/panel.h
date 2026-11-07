#ifndef PANEL_H
#define PANEL_H

#include <stdint.h>

// Function prototypes
uint32_t panel_pressed(uint32_t now, uint32_t before);      /* the switches that went from off to on, one bit per switch */
uint32_t panel_released(uint32_t now, uint32_t before);     /* the switches that went from on to off */
int panel_led(uint32_t *base, int n, int on);               /* turns LED n on or off by read-modify-write, so that
                                                               every other LED keeps its state; −1 if n is not 0 to 31.
                                                               Returns 0, or the iom361 return code of the read or the
                                                               write that failed; after a failed read nothing is written */
int panel_toggle(uint32_t *base, int n);                    /* toggles LED n, the same way, with the same return values */

#endif // PANEL_H