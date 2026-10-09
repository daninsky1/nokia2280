#!/usr/bin/sh

arm-none-eabi-as \
  -march=armv4t \
  -o test.o \
  test.asm
