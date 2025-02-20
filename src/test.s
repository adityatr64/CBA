.global _start

_start:
    mov r0, #0x11
    swi 0x11

    mov r0, #0x00
    mov r7, #1
    swi 0x11

    b .
