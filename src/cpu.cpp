#include "cpu.hpp"
#include <iostream>
#include <stdint.h>
#include "memory.hpp"
#include <ostream>

CPU::CPU(Memory &mem) : memory(mem)
{
  registers.pc = ROM_START;
  registers.cpsr = 0; // Initialize CPSR (Thumb mode is off by default)
  detectThumbInstruction();
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

// Read and Write Register Functions
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

// Check if the CPU is in Thumb mode
bool CPU::isThumbMode() const
{
  return (registers.cpsr & 0x20) != 0; // Check T bit (bit 5)
}

// Decode and execute ARM instructions
void CPU::decodeARM(uint32_t instruction)
{
  std::cout << "ARM Instruction: 0x" << std::hex << instruction << std::endl;

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
  case 0xA000000: // B (Branch)
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
    std::cerr << "Unknown ARM instruction: 0x" << std::hex << instruction << std::endl;
    break;
  }
  }
}

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

// Execute the next instruction
void CPU::executeInstruction()
{
  if (isThumbMode())
  {
    // Thumb mode: 16-bit instruction
    // Nothing works right now
    uint16_t instruction = memory.readHalfWord(registers.pc);
    registers.pc += 2;
    decodeThumb(instruction);
  }
  else
  {
    // ARM mode: 32-bit instruction
    uint32_t instruction = memory.readWord(registers.pc);
    registers.pc += 4;
    decodeARM(instruction);
  }
}

// Run the CPU
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