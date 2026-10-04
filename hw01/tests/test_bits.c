#include <stdio.h>
#include "bits.h"
#include "status.h"

static int fails = 0;
#define CHECK(cond) do { \
    if (cond) printf("PASS %s\n", #cond); \
    else { printf("FAIL %s (line %d)\n", #cond, __LINE__); fails++; } \
} while (0)
 
int main(void) {
    print_binary(0x2C, 8);
    CHECK(get_field(0x1631, 0, 1) == 1);
    CHECK(get_field(0xDEADBEEF, 0, 32) == 0xDEADBEEF);
    CHECK(get_field(0x80000000, 31, 1) == 1);
    CHECK(get_field(0xFF, 30, 3) == 0);
    CHECK(set_field(0, 0, 1, 1) == 1);
    CHECK(set_field(0xDEADBEEF, 0, 32, 0xCAFEBABE) == 0xCAFEBABE);
    CHECK(set_field(0, 31, 1, 1) == 0x80000000);
    CHECK(set_field(0, 4, 3, 0xFF) == 0x70);
    CHECK(set_field(0x1234, 30, 3, 1) == 0);
    CHECK(sign_extend(0x7FFFFFFF, 32) == 2147483647);
    CHECK(sign_extend(0x80, 8) == -128);
    CHECK(sign_extend(1, 1) == -1);
    CHECK(sign_extend(0xF8, 8) == -8);
    CHECK(status_unpack(0x1631).heat == 1);
    /* ...cool, fan, fault, reserved, mode_invalid... */

    status_t s = status_unpack(0x1631);
    CHECK(s.setpoint == 22);
    CHECK(s.mode == MODE_AUTO);
    CHECK(s.heat == 1);

    s = status_unpack(0xF819);
    CHECK(s.setpoint == -8);
    CHECK(s.mode == MODE_HEAT);
    CHECK(s.heat == 1);

    s = status_unpack(0x0070);
    CHECK(s.setpoint == 0);
    CHECK(s.mode == 7);
    CHECK(s.heat == 0);

    CHECK(s.mode_invalid == 1);

    printf("%d failed\n", fails);
    return fails != 0;
}