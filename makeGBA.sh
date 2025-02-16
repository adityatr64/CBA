#!/bin/bash
#this compiles it to little endian ("Arm7tdmi" architecture)

arm-none-eabi-as -mcpu=arm7tdmi src/test.s -o bin/test.o
arm-none-eabi-ld -Tlink.ld  bin/test.o -o bin/test.elf
arm-none-eabi-objcopy -O binary bin/test.elf bin/test.gba 
hexdump -C bin/test.gba | head