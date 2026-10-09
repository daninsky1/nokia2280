#!/usr/bin/sh

arm-none-eabi-ld \
  -T linker.ld \
  -o test.elf \
  test.o
