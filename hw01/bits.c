#include <stdio.h>
#include "bits.h"

void print_binary(uint32_t x, int width){
/*
This function prints a 32-bit unsigned integer in binary format, with a specified width. 
The output is formatted with spaces every 4 bits for readability.
*/
    if (width > WORD_BITS) {
        width = WORD_BITS; 
    }
    for (int i = width - 1; i >= 0; i--) {  /* count down from the top bit so the MSB prints first */
        printf("%u", (x >> i) & 1);

        if (i > 0 && i % NIBBLE_BITS == 0) {
            printf(" "); 
        }
    }
    printf("\n");
}

uint32_t get_field(uint32_t word, int pos, int width){
/*
This function takes a specified portion of a 32-bit word and returns a word shifted to bit 0 
of the specified width, containing the specified portion of the previous word.
*/

    if (width < 1 || width > WORD_BITS || pos < 0 || pos > WORD_BITS-1 || pos + width > WORD_BITS) {
        return 0;  /* invalid width/pos: shift would be undefined or field runs past bit 31; 0 is the chosen error value */
    }

    uint32_t mask = (~0u >> (WORD_BITS - width));  /* all-ones shifted right leaves width ones; shift is 0..31, so width 32 is safe (1u << 32 is not) */
    return (word >> pos) & mask;
}

uint32_t set_field(uint32_t word, int pos, int width, uint32_t value){
/*
This function replaces a specified section of the input word with a new specified value and returns the modified word.
*/

    if (width < 1 || width > WORD_BITS || pos < 0 || pos > WORD_BITS-1 || pos + width > WORD_BITS) {
        return 0;  /* same range check and error value as get_field */
    }

    uint32_t mask = (~0u >> (WORD_BITS - width));  /* width ones in the low bits, same as get_field */
    return (word & ~(mask << pos)) | ((value & mask) << pos);  /* clear the field in word, then OR in value trimmed to width and moved up to pos */

}

int32_t sign_extend(uint32_t value, int width){
/*
This function takes the lowest width bits of value as a two's complement number and return it as an int32_t.
*/

    if (width < 1 || width > WORD_BITS) {  /* only width 1..32 is a valid two's complement field */
        return 0;  /* 0 is the chosen error value, as in get_field */
    }

    uint32_t result = get_field(value, 0, width);

    if ((result >> (width - 1)) & 1) {  /* move the field's top (sign) bit to bit 0 and test it; 1 means negative */
        for (int i = width; i < WORD_BITS; i++) {
            result |= 1u << i;
        }
    }
    return (int32_t)result;

}
