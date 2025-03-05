#!/bin/bash
#this compiles it to little endian ("Arm7tdmi" architecture)

# arm-none-eabi-as -mcpu=arm7tdmi src/kernel.s -o bin/test.o
# arm-none-eabi-ld -Tlink.ld  bin/test.o -o bin/test.elf
# arm-none-eabi-objcopy -O binary bin/test.elf bin/test.gba 
# hexdump -C bin/test.gba | head


arm-none-eabi-gcc -mthumb-interwork -Tlink.ld -nostartfiles -nostdlib -o bin/kernel.elf src/kernel.s
arm-none-eabi-objcopy -O binary bin/kernel.elf bin/kernel.gba
hexdump -C bin/kernel.gba | head