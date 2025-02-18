#include "cpu.hpp"
#include <iostream>
#include <stdint.h>
#include "memory.hpp"
#include <ostream>
#include <bitset>

CPU::CPU(Memory &mem) : memory(mem) // Constructor
{
  for (int i = 0; i < 16; ++i)
  {
    registers.r[i] = 0;
  }
  registers.pc = ROM_START;
  registers.cpsr = 0; // Initialize CPSR (Thumb mode is off by default)
}

void CPU::updateFlags(uint32_t result, bool carry, bool overflow)
{
  registers.cpsr &= ~(0xF << 28); // Clear N, Z, C, V flags (bits 28-31)

  if (result == 0) // Zero Flag (Z)
    registers.cpsr |= (1 << 30);

  if (result & 0x80000000) // Negative Flag (N)
    registers.cpsr |= (1 << 31);

  if (carry) // Carry Flag (C)
    registers.cpsr |= (1 << 29);

  if (overflow) // Overflow Flag (V)
    registers.cpsr |= (1 << 28);
}

void CPU::detectThumbInstruction()
{
  // Check the first instruction in ROM to determine if it's a Thumb instruction
  uint16_t firstInstruction = memory.readHalfWord(ROM_START);

  // If the first instruction is a valid Thumb instruction, set the T bit in CPSR
  if ((firstInstruction & 0xF800) == 0xE000 || // B (unconditional branch)
      (firstInstruction & 0xF800) == 0x4800 || // LDR (literal)
      (firstInstruction & 0xF800) == 0x4000)   // AND/OR/XOR/LSL/LSR/ASR
  {
    registers.cpsr |= 0x20; // Set T bit to enter Thumb mode
  }
  else
  {
    registers.cpsr &= ~0x20; // Clear T bit to enter ARM mode
  }
}
// complex Register handling functions
uint32_t CPU::readRegister(int index) const
{
  if (index < 0 || index > 15)
  {
    return 0;
  }
  return registers.r[index];
}
void CPU::writeRegister(int index, uint32_t value)
{
  if (index < 0 || index > 15)
  {
    std::cout << "Invalid register index: " << index << std::endl;
    return;
  }
  registers.r[index] = value;
}

// Execute the next instruction
void CPU::executeInstruction()
{
  // Inline isThumbMode()
  if ((registers.cpsr & 0x20) != 0)
  {
    // Thumb mode: 16-bit instruction
    uint16_t instruction = memory.readHalfWord(registers.pc);
    registers.pc += 2;
    decodeThumb(instruction);
  }
  else
  {
    // ARM mode: 32-bit instruction
    // The ARM instruction is always word-aligned, so we can read 4 bytes directly
    // The functions are defined in arm.cpp
    uint32_t instruction = memory.readWord(registers.pc);
    registers.pc += 4;
    decodeARM(instruction);
  }
}

// Run the CPU
void CPU::run()
{
  detectThumbInstruction();
  std::cout << "ROM Size: " << memory.getROMSize() << std::endl;
  memory.dumpROM();
  for (;;)
  {
    executeInstruction();
    std::cout << "r0: " << std::hex << registers.r[0] << std::endl;
    std::cout << "r1: " << std::hex << registers.r[1] << std::endl;
    std::cout << "r2: " << std::hex << registers.r[2] << std::endl;
    std::cout << "r3: " << std::hex << registers.r[3] << std::endl;
    std::cout << "r4: " << std::hex << registers.r[4] << std::endl;
    std::cout << "r5: " << std::hex << registers.r[5] << std::endl;
    std::cout << "CPSR: " << std::bitset<32>(registers.cpsr) << std::endl;
    std::cout << "N (Negative): " << ((registers.cpsr >> 31) & 1) << std::endl;
    std::cout << "Z (Zero): " << ((registers.cpsr >> 30) & 1) << std::endl;
    std::cout << "C (Carry): " << ((registers.cpsr >> 29) & 1) << std::endl;
    std::cout << "V (Overflow): " << ((registers.cpsr >> 28) & 1) << std::endl;
    std::cout << "T (Thumb mode): " << ((registers.cpsr >> 5) & 1) << std::endl;
  }
  std::cout << "\n\n----Reached END----\n\n";
}