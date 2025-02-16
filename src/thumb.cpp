#include "cpu.hpp"
#include <iostream>
#include "memory.hpp"
/*
I've split this to keep too much code accumulation in one file
This file will contain the implementation of the Thumb CPU class methods.
*/
// Decode and execute Thumb instructions
void CPU::decodeThumb(uint16_t instruction)
{
  std::cout << "Thumb Instruction: 0x" << std::hex << instruction << std::endl;

  uint16_t opcode = (instruction >> 10) & 0x3F; // Common opcode extraction

  std::cout << "Opcode: 0x" << std::hex << opcode << std::endl;

  switch (opcode)
  {
  case 0x12: // Thumb MOV Rd, #imm (8-bit)
  {
    uint32_t reg = (instruction >> 8) & 0x7;
    uint32_t imm = instruction & 0xFF;
    writeRegister(reg, imm);
    break;
  }
  case 0x20: // Thumb ADD Rd, Rs, Rn
  {
    uint32_t destReg = (instruction >> 0) & 0x7;
    uint32_t sourceReg = (instruction >> 3) & 0x7;
    uint32_t addReg = (instruction >> 6) & 0x7;
    writeRegister(destReg, readRegister(sourceReg) + readRegister(addReg));
    break;
  }
  case 0x38: // Thumb B (conditional branch)
  {
    int32_t offset = (instruction & 0x7FF) << 1; // Extract 11-bit offset, multiply by 2
    offset = (offset << 21) >> 21;               // Sign-extend the 12-bit offset
    registers.pc += offset;
    registers.pc -= 2; // Undo the increment from the loop
    break;
  }
  case 0x24: // LDR (literal)
  {
    uint32_t reg = (instruction >> 8) & 0x7;
    uint32_t imm = (instruction & 0xFF) << 2; // 8-bit immediate value, shifted left by 2
    uint32_t address = (registers.pc & 0xFFFFFFFC) + imm;
    uint32_t value = memory.readWord(address);
    writeRegister(reg, value);
    break;
  }
  default:
  {
    std::cerr << "Unknown Thumb instruction: 0x" << std::hex << instruction << std::endl;
    break;
  }
  }
}