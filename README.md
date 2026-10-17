
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


## Bit Twiddling Hacks
https://graphics.stanford.edu/~seander/bithacks.html
