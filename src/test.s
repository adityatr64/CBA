.global _start

_start:
    ldr r1, =0x69           @ Load 0x69 into r1
    ldr r0, =addr           @ Load address 0x02000000 into r0 (using = for address)
    str r1, [r0]            @ Store r1 into memory
    ldr r2, [r0]            @ Load value from memory into r2

    b .                     @ Infinite loop
    .align 4
addr:
    .word 0x02000000        @ Literal pool (not needed anymore)
