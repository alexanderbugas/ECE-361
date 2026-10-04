# HOMEWORK 1

- choice made to truncate input at 32-bits rather than return void for print_binary function

- chose to make out of range return 0 to be consistent with the return value of all out of bounds or invalid input

## Design decisions

### `print_binary`
- A `width` above 32 is clamped to 32, so all 32 bits print.
- A `width` of 0 or less prints nothing but the newline.
- Bits are grouped in fours counted from bit 0, so a partial group appears on the left without padding: `print_binary(0x2C, 6)` prints `10 1100`.
- Output ends with a newline.

### `get_field` and `set_field`
- Valid input: `width` 1 to 32, `pos` 0 to 31, and `pos + width` at most 32.
- Any other input returns 0. For `set_field` this means an invalid call returns 0, not the original word.
- The mask is built as `~0u >> (32 - width)`, so it never shifts by 32 and width 32 works.
- `set_field` keeps only the lowest `width` bits of `value`; extra high bits are dropped.

### `sign_extend`
- Valid input: `width` 1 to 32. Any other width returns 0.
- Bits of `value` above the field are ignored.

### `status_unpack`
- An invalid mode (5 to 7) is reported by setting `mode_invalid` to 1. `mode` still holds the raw value so the caller can see it.
- The set point is sign-extended from 8 bits, so it ranges from -128 to 127.
- The reserved bit is reported as read. It is not checked against 0.

## Behavior at the boundaries
- **Width 1:** the mask is a single 1, so `get_field` and `set_field` work on one bit. `sign_extend(v, 1)` returns 0 or -1, the only values a 1-bit two's complement number can hold.
- **Width 32 (pos must be 0):** the mask `~0u >> 0` is all ones, so `get_field` returns the whole word and `set_field` returns `value`. In `sign_extend` the fill loop does not run because no bits sit above the field; the cast to `int32_t` gives the result.
- **Pos 31 (width must be 1):** `get_field` returns bit 31 using `word >> 31`, which is legal. Width 2 at pos 31 is out of range and returns 0.
- **Most negative value:** `sign_extend(0x80, 8)` returns -128 by copying the sign bit into bits 8 to 31. The code never negates, so -128 works even though +128 does not fit in 8 bits. At width 32, `sign_extend(0x80000000, 32)` returns -2147483648.

## What the library does
- `bits.c` / `bits.h`: four bit-manipulation functions for 32-bit words. `print_binary` prints the low bits of a value in groups of four. `get_field` reads a field of bits. `set_field` writes a field of bits. `sign_extend` reads a field as a two's complement number.
- `status.c` / `status.h`: `status_unpack` decodes a 16-bit thermostat status word into a `status_t` struct, using `get_field` and `sign_extend` with named constants for every field position and width.

## Build and test
Run these from `hw01/`:
- `make` compiles `bits.c` and `status.c` to `bits.o` and `status.o`.
- `make test` builds `tests/test_bits` from the objects and runs it. It prints PASS or FAIL for each check, then the number of failures, and exits nonzero if any check fails.
- `make clean` removes the object files and the test program.
