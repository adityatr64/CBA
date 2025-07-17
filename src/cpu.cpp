#include "../include/cpu.hpp"

#include <stdint.h>

#include <bitset>
#include <iostream>
#include <ostream>

#include "../include/arm.hpp"  // Include ARM namespace
#include "../include/memory.hpp"

CPU::CPU(Memory &mem)
    : memory(mem)  // Constructor
{
  for (int i = 0; i < 16; ++i) {
    registers.r[i] = 0;
  }
  registers.pc = ROM_START;
  registers.cpsr = 0x00000010;  // Initialize CPSR as User (Thumb mode is off by default)
  registers.spsr = 0;
}

void CPU::updateFlags(uint32_t result, bool carry, bool overflow) {
  registers.cpsr &= ~(0xF << 28);  // Clear N, Z, C, V flags (bits 28-31)
  if (result == 0)                 // Zero Flag (Z)
    registers.cpsr |= (1 << 30);
  if (result & 0x80000000)  // Negative Flag (N)
    registers.cpsr |= (1 << 31);
  if (carry)  // Carry Flag (C)
    registers.cpsr |= (1 << 29);
  if (overflow)  // Overflow Flag (V)
    registers.cpsr |= (1 << 28);
}

void CPU::detectThumbinst() {
  uint16_t firstinst = memory.readHalfWord(ROM_START);

  // If the first inst is a valid Thumb inst, set the T bit in CPSR
  if ((firstinst & 0xF800) == 0xE000 ||  // B (unconditional branch)
      (firstinst & 0xF800) == 0x4800 ||  // LDR (literal)
      (firstinst & 0xF800) == 0x4000)    // AND/OR/XOR/LSL/LSR/ASR
  {
    registers.cpsr |= 0x20;  // Set T bit to enter Thumb mode
  } else {
    registers.cpsr &= ~0x20;  // Clear T bit to enter ARM mode
  }
}

// Register handling functions
uint32_t CPU::readRegister(int index) const {
  if (index < 0 || index > 15) {
    return 0;
  }
  return registers.r[index];
}

void CPU::writeRegister(int index, uint32_t value) {
  if (index < 0 || index > 15) {
    std::cout << "Invalid register index: " << index << std::endl;
    return;
  }
  registers.r[index] = value;
}

void CPU::executeinst() {
  if ((registers.cpsr & 0x20) != 0) {
    // Thumb mode: 16-bit inst
    uint16_t inst = memory.readHalfWord(registers.pc);
    registers.pc += 2;
    decodeThumb(inst);
  } else {
    // ARM mode: 32-bit inst
    // The ARM inst is always word-aligned, so we can read 4 bytes directly
    // The functions are defined in arm.cpp
    uint32_t inst = memory.readWord(registers.pc);
    registers.pc += 4;
    ARM::decodeARM(this, &memory, inst);
  }
}

// Run the CPU
void CPU::run() {
  detectThumbinst();
  std::cout << "ROM Size: " << memory.getROMSize() << std::endl;
  memory.dumpROM();
  for (;;) {
    executeinst();
    std::cout << "r0: " << std::hex << registers.r[0] << std::endl;
    std::cout << "r1: " << std::hex << registers.r[1] << std::endl;
    std::cout << "r2: " << std::hex << registers.r[2] << std::endl;
    std::cout << "r3: " << std::hex << registers.r[3] << std::endl;
    std::cout << "r4: " << std::hex << registers.r[4] << std::endl;
    std::cout << "r5: " << std::hex << registers.r[5] << std::endl;
    std::cout << "r6: " << std::hex << registers.r[6] << std::endl;
    std::cout << "CPSR: " << std::bitset<32>(registers.cpsr) << std::endl;
    std::cout << "N (Negative): " << ((registers.cpsr >> 31) & 1) << std::endl;
    std::cout << "Z (Zero): " << ((registers.cpsr >> 30) & 1) << std::endl;
    std::cout << "C (Carry): " << ((registers.cpsr >> 29) & 1) << std::endl;
    std::cout << "V (Overflow): " << ((registers.cpsr >> 28) & 1) << std::endl;
    std::cout << "T (Thumb mode): " << ((registers.cpsr >> 5) & 1) << std::endl;
  }
  std::cout << "\n\n----Reached END----\n\n";
}