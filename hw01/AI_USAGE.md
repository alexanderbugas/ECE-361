# AI Usage — Homework 1

Document produced with AI and edited by me, Alex.

## Tools used

- **Claude Code** (Anthropic, Claude Opus 5.5), running in the VS Code extension.
- **VS Code inline AI autocomplete**, which suggests code as I type (see entry 3).

## Log

### 1. Where to put the Part 2 functions (`bits.h` vs. `bits.c`)

**What I asked:** The assignment says to "write `bits.h` and `bits.c` with the four functions" in Table 1. Which parts go in which file, and how do I know what goes inside each function?

**What the AI explained:**

- **`bits.h` holds the declarations (prototypes).** Each signature is copied exactly from the left column of Table 1 and ends with a semicolon, e.g.  `void print_binary(uint32_t x, int width);`. The header has to include `<stdint.h>` because the prototypes use `uint32_t` and `int32_t`. All  prototypes go inside the include guard, between `#define BITS_H` and `#endif`.

- **`bits.c` holds the definitions (the bodies).** It includes `<stdio.h>` (for `printf` in `print_binary`) and `"bits.h"`, then repeats each signature without the semicolon, followed by `{ ... }` with the implementation.

- **The right column of Table 1 is the spec for each body.** For example,  `get_field` must return bits `pos` to `pos+width-1` of `word`, shifted down to bit 0. The paragraph under the table adds two more requirements: choose and document a behavior for out-of-range `pos`/`width`, and handle `width == 32` correctly.

- The header is the contract (names, parameters, return types). The `.c` file is the implementation. Other files (`status.c`, `tests/test_bits.c`) include `bits.h` to call the functions without seeing their code.

**What I did with it:** I wrote the prototypes in `bits.h` myself. For this question the AI explained the file structure only. Later entries note where it supplied code.

### 2. `print_binary`

When I asked how to approach the initial empty code project, the AI explained about parameters and after I gave a shot at my own pseudocode, the AI gave updated pseudocode with snippets of proper syntax, for a loop counting down from bit `width - 1`, along with the `(x >> i) & 1` idiom for extracting one bit. When I asked why the loop prints MSB first, what `& 1` does, and why `i` was flagged as undefined, it walked through the shifts bit by bit and showed that my space check had to move inside the loop because `i` only exists in the loop's scope. It also suggested clamping widths above 32 to avoid undefined shifts and tested my finished function at the boundary widths.

### 3. What a model got wrong: autocomplete in `get_field`

While I was writing a bit-by-bit loop in `get_field`, my editor's AI autocomplete suggested `if ((word >> (pos + t)) & 1) { return 1 << t; }`. I accepted the suggestion and then asked how the return value is effected; it would returns as soon as it finds the first 1-bit, so only one bit of the field comes back. It was found it by tracing `get_field(0x1631, 4, 3)`, which should return 3 (`011`) but would return 1. I rejected the suggestion and used a single shift-and-mask instead of the loop.

### 4. `get_field` mask edge case

My first mask was `int mask = (1 << width) - 1;`. When I asked whether it produces `width` ones, the AI explained that it does for widths 1 to 30, but `1 << 31` overflows a signed `int` and `1 << 32` shifts by the full width of the type, so widths 31 and 32 are undefined behavior. It suggested making the mask unsigned (`1u`, `uint32_t`), which fixes width 31, and handling width 32 either as a special case or by building the mask as `~0u >> (32 - width)`, which only ever shifts by 0 to 31.

### 5. `sign_extend`

When I didn't understand how `sign_extend(0xF8, 8)` gives -8, the AI explained two's complement (the top bit of the field has negative weight) and why the field's sign bit has to be copied into the upper bits of the 32-bit result. I proposed reusing `get_field(value, 0, width)` to clear the bits above the field, and the AI confirmed it. It then provided the code structure I adapted: check the sign bit once with `(result >> (width - 1)) & 1`, set bits `width` through 31 with `result |= 1u << i` if it is negative, and return `(int32_t)result` after the loop. It also pointed out that my draft used `1 << i` (signed overflow at bit 31) and an `int32_t` working variable, and that the function had no return statement.

### 6. `set_field`

I wrote `set_field` myself and AI helped with the line 46 logic implementation when i failed to catch all edge cases. The AI compiled it and ran 12 test cases (width 1 and 32, pos 31, a value too wide for its field, out-of-range input, and a round trip through `get_field`), all of which passed with no undefined behavior. It explained my return line in four parts and raised two design points for later: returning 0 on invalid input wipes the caller's word, and the range check is repeated in `get_field`.

### 7. `status.h`

My first draft mixed bit positions with MODE values (for example `MODE_OFF 5`) and had `FAULT` at bit 4 instead of 3. The AI pointed out that `status.c` needs a position and width for each field from Table 2, that the MODE values 0 to 4 are a separate set of constants, and that the struct was missing the reserved field. At my request it rewrote the constants and the struct with those changes (lowercase member names, a `reserved` member, and my invalid-mode flag renamed to `mode_invalid`). It then explained where each number comes from in Table 2.

### 8. `status.c`

I wrote `status_unpack` myself. When I asked why MODE has no constants for 5 to 7, the AI explained that the 3-bit field can only hold 0 to 7, so anything above `MODE_MAX_VALID` is invalid without naming each value. It pointed out two bugs in my draft: the set point needed `sign_extend` (otherwise -8 °C reads as 248), and the function had no `return`. After I fixed them it ran `status_unpack(0x1631)` and other words, and the results matched the spec.

### 9. `Makefile`

Because of time constraints, the AI wrote the whole Makefile from the pattern on the week 1 slides. Changes from the slide: plain `make` builds `bits.o` and `status.o` (there is no `main` to link), a `test` target links `tests/test_bits` with both objects and runs it, the test file is compiled with `-I.` so it can find the headers in `hw01/`, and `clean` also removes the test program. The AI checked it with `make`, `make test`, and `make clean`.

### 10. `tests/test_bits.c`

I wrote the tests starting from the `CHECK` macro on the week 1 slides. The AI explained why including `bits.c` and `status.c` caused "multiple definition" link errors (headers declare, `.c` files define, and the linker joins the objects), gave a list of boundary cases that cover the assignment, and pointed out mistakes in my drafts: `count_ones` does not exist in my code, `sign_extend(0x90, 8)` is -112 so the most negative test needs `0x80`, `print_binary` returns `void` so it cannot go inside `CHECK`, and the `status_t s` variable had to be declared before use.
