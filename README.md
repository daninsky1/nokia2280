
A3.1 Instruction set encoding
p. 110 - Tabela de tipos de instruções

p. 112 - Lista de condition opcodes

p. 112 - Lista de branch instructions

p. 115 - Lista de data-processing instruction opcodes

p. 121 - Lista de multiply instructions




Improved instruction decoding.
Reorganized instruction decoding constants and function..
All types of instructions follows its own path. Not all are decoded yet,
Added implementation of data processing instruction decoding.
Added helper mnemonics constants, names and description and helper functions.

## Signed overflow (V flag)

P means sign bit 0 (zero or positive); N means sign bit 1 (negative).
RES is the sign of the 32-bit result.

### Addition: `OP1 + OP2`

| OP1 | OP2 | RES | V |
| --- | --- | --- | --- |
| P | P | P | 0 |
| P | P | N | 1 |
| N | N | N | 0 |
| N | N | P | 1 |
| P | N | Any | 0 |
| N | P | Any | 0 |

For addition: `V = (sign(OP1) == sign(OP2)) && (sign(RES) != sign(OP1))`.

### Subtraction: `OP1 - OP2`

| OP1 | OP2 | RES | V |
| --- | --- | --- | --- |
| P | P | Any | 0 |
| N | N | Any | 0 |
| P | N | P | 0 |
| P | N | N | 1 (above maximum) |
| N | P | N | 0 |
| N | P | P | 1 (below minimum) |

For subtraction: `V = (sign(OP1) != sign(OP2)) && (sign(RES) != sign(OP1))`.

### Why the formulas work

First, use signed 8-bit numbers: they range from `-2^7 = -128` to
`2^7 - 1 = 127`. The result keeps only 8 bits, so values outside that range
wrap around:

| Operation | Exact answer | Stored 8 bits | Read as signed | V |
| --- | ---: | --- | ---: | ---: |
| `127 + 1` | `128` (too high) | `1000 0000` | `-128` | 1 |
| `-128 + (-1)` | `-129` (too low) | `0111 1111` | `127` | 1 |
| `127 - (-1)` | `128` (too high) | `1000 0000` | `-128` | 1 |
| `-128 - 1` | `-129` (too low) | `0111 1111` | `127` | 1 |

ARM uses the same rule with 32 bits: the signed range is `-2^31` to
`2^31 - 1`, and the result keeps the low 32 bits.

- Addition of opposite-sign operands stays between them, so it cannot overflow.
  With matching signs, a sign change means two nonnegative values went above the
  maximum, or two negative values went below the minimum.
- Subtraction of same-sign operands stays within the signed range. With opposite
  signs, a result sign different from OP1 means subtracting a negative value went
  above the maximum, or subtracting a nonnegative value went below the minimum.

## SBC carry flag (no borrow)

ARM DDI 0100I, A4.1.65 defines `SBC` as `OP1 - OP2 - NOT(C_in)`. Here `C_in`
is the old CPSR.C flag, and `OP2` is the shifter operand. The new C flag is 1
when this subtraction needs no unsigned borrow.

Let `borrow_in = 1 - C_in`. No borrow means `OP1 >= OP2 + borrow_in`, with the
right side considered at full precision. Splitting this into the two possible
values of `C_in` gives:

| Old C (`C_in`) | Subtraction | New C is 1 when |
| ---: | --- | --- |
| 1 | `OP1 - OP2` | `OP1 >= OP2` |
| 0 | `OP1 - OP2 - 1` | `OP1 > OP2` |

So the 32-bit C++ calculation is:

```cpp
const bool carryIn = (cpu.cpsr() & PSR::C) != 0;
const bool carryOut = (operand1 > operand2) ||
                      (operand1 == operand2 && carryIn);
```

For a small visual example, imagine 8-bit unsigned operands. The stored result
keeps only the low 8 bits, but C still tells whether a borrow was needed:

| OP1 | OP2 | Old C | Exact subtraction | Stored 8 bits | New C |
| ---: | ---: | ---: | --- | --- | ---: |
| 5 | 5 | 1 | `5 - 5 - 0 = 0` | `0000 0000` | 1 |
| 5 | 5 | 0 | `5 - 5 - 1 = -1` | `1111 1111` | 0 |
| 6 | 5 | 0 | `6 - 5 - 1 = 0` | `0000 0000` | 1 |
| 0 | 255 | 0 | `0 - 255 - 1 = -256` | `0000 0000` | 0 |

The last row shows why `OP2 + borrow_in` must not be computed in the operand's
width: `255 + 1` would wrap to zero in 8 bits, hiding the borrow.


## Bit Twiddling Hacks
https://graphics.stanford.edu/~seander/bithacks.html
