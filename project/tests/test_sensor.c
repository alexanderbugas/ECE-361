/*
 * test_sensor.c: unit tests for sensor.h / sensor.c (from HW6)
 *
 * The sheet's conversion tests (0 C, 100 C, a threshold) and HW6's two.
 */
#include "check.h"
#include "iom361_r4.h"
#include "sensor.h"

void test_sensor(void) {
    int rc;
    uint32_t *base = iom361_initialize(1, 2, &rc);              /* the vent's 1 switch and 2 LEDs */
    reading_t r;

    iom361_set_display(false);                                  /* no LED lines between the test lines */
    check(base != NULL, "sensor: iom361_initialize works");

    _iom361_set_sensor1(0.0f, 50.0f);                           /* the sheet: 0 C */
    check(sensor_read(base, 0, &r) == 0 && r.temp_c == 0.0f, "sensor: 0.0 C reads back as 0.0");
    _iom361_set_sensor1(100.0f, 50.0f);                         /* the sheet: 100 C */
    check(sensor_read(base, 0, &r) == 0 && r.temp_c == 100.0f, "sensor: 100.0 C reads back as 100.0");
    _iom361_set_sensor1(28.0f, 85.0f);                          /* the sheet: a threshold of the rules */
    check(sensor_read(base, 0, &r) == 0 && r.temp_c == 28.0f && r.rh_pct == 85.0f,
          "sensor: the thresholds 28.0 C and 85.0 %RH read back exactly");
    _iom361_set_sensor1(91.0f, 40.0f);                          /* HW6: the register holds 90.99998 */
    check(sensor_read(base, 1791273600, &r) == 0 && r.temp_c == 91.0f && r.t == 1791273600,
          "sensor: 91.0 C reads back as 91.0, with the timestamp");
    _iom361_set_sensor1(-12.3f, 0.0f);
    check(sensor_read(base, 0, &r) == 0 && r.temp_c == -12.3f, "sensor: -12.3 C reads back as -12.3");

    uint32_t not_base[8] = { 0 };                               /* HW6: a failed read changes nothing */
    reading_t kept = { .t = 7, .temp_c = 1.5f, .rh_pct = 2.5f };
    check(sensor_read(not_base, 99, &kept) == 1, "sensor: a wrong base returns rc 1");
    check(kept.t == 7 && kept.temp_c == 1.5f && kept.rh_pct == 2.5f, "sensor: a failed read changes nothing");
}
