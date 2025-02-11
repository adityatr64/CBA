.global _start

@ .syntax unified

_start:
    MOVs r0, #0x10       @ Load 0x10 into r0
    b .
.align 4
