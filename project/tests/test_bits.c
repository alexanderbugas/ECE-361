/*
 * test_bits.c: unit tests for bits.h / bits.c (copied unchanged from HW1)
 *
 * Only get_field and set_field are used by the project, at the positions of
 * the status word, so those are what is tested here.
 */
#include "check.h"
#include "bits.h"

/*
 * set_field writes one field and keeps the others; get_field reads it back
 *
 * Why: the status word is packed one field at a time, so each set_field must
 * leave the fields already written alone.
 */
static void test_fields(void) {
    uint32_t w = 0;

    w = set_field(w, 4, 3, 4);                  /* state field, STORM_LOCK */
    w = set_field(w, 2, 1, 1);                  /* wind bit */
    check(w == 0x44u, "bits: set state 4 at bit 4 and bit 2 gives 0x44");
    check(get_field(w, 4, 3) == 4u, "bits: get the 3-bit field at bit 4");
    check(get_field(w, 2, 1) == 1u, "bits: get the 1-bit field at bit 2");

    w = set_field(w, 4, 3, 1);                  /* overwrite the state */
    check(w == 0x14u, "bits: overwriting a field keeps the others");
    check(set_field(0, 4, 3, 0xFFu) == 0x70u, "bits: a value wider than its field is cut to the field");
    check(get_field(0xFFFFFFFFu, 32, 1) == 0u, "bits: a position past bit 31 gives 0");
}

void test_bits(void) {
    test_fields();
}
