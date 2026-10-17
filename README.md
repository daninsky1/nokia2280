
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

How to calculate oVerflow:
oVerflow:
OP1  OP2 RES V
P    P   P   0
P    P   N   1
N    N   N   0
N    N   P   1
P    N   Any 0
N    P   Any 0


## Bit Twiddling Hacks
https://graphics.stanford.edu/~seander/bithacks.html