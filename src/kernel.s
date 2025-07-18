.global _start
.arm
.section .text
.align 4
_start:
    MOV  R0, #0          @ Set R0 = 0 (to trigger Z flag)
    CMP  R0, #0          @ Compare 0 - 0 → Z = 1
    MOV R1, #0x70000000
    ADDS R1, R1, R1
    MRS  R2, CPSR        @ Save flags into R2
    MOV  R3, #0x10       @ Control field value (e.g., User mode = 0x10)
    ORR  R3, R3, #0xC0000000 @ Set Z and V bits in flags (Z = bit 30, V = bit 28)
    MSR  CPSR_fc, R3     @ Write flags + control field to CPSR
    MRS  R4, CPSR        @ Save new CPSR into R4
    B .                  @ Infinite loop
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
