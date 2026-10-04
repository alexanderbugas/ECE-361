#include <stdio.h>
#include "bits.h"

void print_binary(uint32_t x, int width){
/*
This function prints a 32-bit unsigned integer in binary format, with a specified width. The output is formatted with spaces every 4 bits for readability.
*/
    if (width > WORD_BITS) {
        width = WORD_BITS; 
    }
    for (int i = width - 1; i >= 0; i--) {
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
        return 0; 
    }

    uint32_t mask = (~0u >> (WORD_BITS - width));
    return (word >> pos) & mask;
}

uint32_t set_field(uint32_t word, int pos, int width, uint32_t value){
/*
This function replaces a specified section of the input word with a new specified value and returns the modified word.
*/

    if (width < 1 || width > WORD_BITS || pos < 0 || pos > WORD_BITS-1 || pos + width > WORD_BITS) {
        return 0; 
    }

    uint32_t mask = (~0u >> (WORD_BITS - width));
    return (word & ~(mask << pos)) | ((value & mask) << pos);

}

int32_t sign_extend(uint32_t value, int width){
/*
This function takes the lowest width bits of value as a two's complement number and return it as an int32_t.
*/

    if (width < 1 || width > WORD_BITS) {
        return 0;
    }

    uint32_t result = get_field(value, 0, width);

    if ((result >> (width - 1)) & 1) {
        for (int i = width; i < WORD_BITS; i++) {
            result |= 1u << i;
        }
    }
    return (int32_t)result;

}
