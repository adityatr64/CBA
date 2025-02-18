.global _start

_start:

    ldr r1, =0x02000008  @ Load the address directly
    ldr r2, =0xABCDEF00  @ Valid 32-bit value
    str r2, [r1]

    b .  @ Infinite loop
