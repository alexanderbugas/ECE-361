#include "status.h"

/*
Strust status_t is from the status.h file. It breaks down the 32-bit word into individual status fields.
*/

status_t status_unpack(uint16_t word){
    status_t status;

    status.heat = get_field(word, HEAT_POS, FLAG_WIDTH);
    status.cool = get_field(word, COOL_POS, FLAG_WIDTH);
    status.fan = get_field(word, FAN_POS, FLAG_WIDTH);
    status.fault = get_field(word, FAULT_POS, FLAG_WIDTH);
    status.mode = get_field(word, MODE_POS, MODE_WIDTH);
    status.reserved = get_field(word, RESERVED_POS, FLAG_WIDTH);
    status.setpoint = sign_extend(get_field(word, SETPOINT_POS, SETPOINT_WIDTH), SETPOINT_WIDTH);
    
    if (status.mode > MODE_MAX_VALID) {
        status.mode_invalid = 1;
    } else {
        status.mode_invalid = 0;
    }
    return status;
}