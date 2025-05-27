.global _start
.arm
.section .text
.align 4
_start:
    MOV r1, #10

.section .data
.align 4
data:
    .word 0x12345678

.align 4 @ Forces byte_data to start at a 4-byte boundary
byte_data:
    .byte 0x55

.align 4 @ Ensures new_value starts at a 4-byte boundary
halfword_data:
    .hword 0x1234
.align 4
new_value:
    .word 0xAABBCCDD

.end
