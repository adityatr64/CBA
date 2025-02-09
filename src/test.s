.global _start

@ .syntax unified
@ .thumb                  @ Use Thumb mode

_start:
    ldr r1, =0x69       @ Load 0x69 into r1 (Thumb mode uses `ldr` for large immediates)
    ldr r0, =addr       @ Load address 0x02000000 into r0
    str r1, [r0]        @ Store r1 into memory at address in r0
    ldr r2, [r0]        @ Load value from memory into r2

    b .                 @ Infinite loop

.align 4
addr:
    .word 0x02000000    @ Literal pool for address
