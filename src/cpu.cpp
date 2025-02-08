#include "cpu.hpp"
#include <iostream>
#include <stdint.h>
#include "memory.hpp"
#include <ostream>

CPU::CPU(Memory &mem) : memory(mem)
{
  registers.pc = ROM_START;
}

// Very Complex Read and Write Register Functions Dont mess with it

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

void CPU::executeInstruction()
{
  uint32_t instruction = memory.readWord(registers.pc);
  registers.pc += 4;

  std::cout << "Instruction: 0x" << std::hex << instruction << std::endl;

  switch (instruction & 0xF000000)
  {
    // TODO: Implement the remaining instructions
    // TODO: Implement the Sub cases for the instructions

  case 0x3000000: // MOV
  {
    uint32_t reg = (instruction >> 12) & 0xF;
    uint32_t imm = instruction & 0xFFF;
    writeRegister(reg, imm);
    break;
  }
  case 0x000000: // ADD
  {
    uint32_t destReg = (instruction >> 12) & 0xF;
    uint32_t sourceReg = (instruction >> 8) & 0xF;
    uint32_t addReg = instruction & 0xF;
    writeRegister(destReg, readRegister(sourceReg) + readRegister(addReg));
    break;
  }
  case 0xA000000: // B
  {
    int32_t offset = (instruction & 0xFFFFFF) << 2; // Extract 24-bit offset, multiply by 4
    offset = (offset << 6) >> 6;                    // Sign-extend the 26-bit offset
    registers.pc += offset;
    registers.pc -= 4; // Undo the increment from the loop
    break;
  }
  case 0x5000000: // LDR/STR
  {
    uint32_t opcode = (instruction >> 20) & 0xFF; // Extract bits 27-20
    uint32_t baseReg = (instruction >> 16) & 0xF; // Base register (Rn)
    uint32_t destReg = (instruction >> 12) & 0xF; // Destination/source register (Rd)
    uint32_t offset = instruction & 0xFFF;        // Immediate offset

    uint32_t address = readRegister(baseReg) + offset;

    if ((opcode & 0xF0) == 0x50)
    {
      if ((opcode & 0x0F) == 0x09) // LDR (0x59)
      {
        uint32_t value = memory.readWord(address);
        writeRegister(destReg, value);
      }
      else if ((opcode & 0x0F) == 0x08) // STR (0x58)
      {
        uint32_t value = readRegister(destReg);
        memory.writeWord(address, value);
      }
      else
      {
        std::cerr << "Unknown LDR/STR variant: 0x" << std::hex << instruction << std::endl;
      }
    }
    else
    {
      std::cerr << "Unknown instruction: 0x" << std::hex << instruction << std::endl;
    }
    break;
  }
  default:
  {
    std::cerr << "Unknown instruction: 0x" << std::hex << instruction << std::endl;
    break;
  }
  }
}

void CPU::run()
{
  std::cout << "ROM Size: " << memory.getROMSize() << std::endl;
  memory.dumpROM();
  for (size_t i = 0;; ++i)
  {
    executeInstruction();
    std::cout << "r0: " << std::hex << registers.r[0] << std::endl;
    std::cout << "r1: " << std::hex << registers.r[1] << std::endl;
    std::cout << "r2: " << std::hex << registers.r[2] << std::endl;
    std::cout << "r3: " << std::hex << registers.r[3] << std::endl;
    std::cout << "r4: " << std::hex << registers.r[4] << std::endl;
    std::cout << "r5: " << std::hex << registers.r[5] << std::endl;
  }
  std::cout << "\n\n----Reached END----\n\n";
}