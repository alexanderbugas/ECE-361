#ifndef STATUS_H
#define STATUS_H
#include "bits.h"

/* field positions and widths (Table 2) */
#define FLAG_WIDTH      1
#define HEAT_POS        0
#define COOL_POS        1
#define FAN_POS         2
#define FAULT_POS       3
#define MODE_POS        4
#define MODE_WIDTH      3
#define RESERVED_POS    7
#define SETPOINT_POS    8
#define SETPOINT_WIDTH  8

/* MODE field values */
#define MODE_OFF        0
#define MODE_HEAT       1
#define MODE_COOL       2
#define MODE_AUTO       3
#define MODE_FAN_ONLY   4
#define MODE_MAX_VALID  4

typedef struct {
/* one member per field*/
    int heat;
    int cool;
    int fan;
    int fault;
    int mode;
    int reserved;
    int setpoint;
    int mode_invalid;
} status_t;

status_t status_unpack(uint16_t word);

#endif /* STATUS_H */
