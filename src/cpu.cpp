#include "cpu.hpp"
#include <iostream>
#include <stdint.h>
#include "memory.hpp"
#include <ostream>

CPU::CPU(Memory &mem) : memory(mem)
{
  for (int i = 0; i < 16; ++i)
  {
    registers.r[i] = 0;
  }
  registers.pc = ROM_START;
  registers.cpsr = 0; // Initialize CPSR (Thumb mode is off by default)
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

// Decode and execute ARM instructions
void CPU::decodeARM(uint32_t instruction)
{
  std::cout << "ARM Instruction: 0x" << std::hex << instruction << std::endl;

  // Condition check [COND] part of the instruction
  uint32_t condition = (instruction >> 28) & 0xF; // Extract condition field (bits 31-28)
  bool execute = false;

  switch (condition)
  {
  case 0x0:                                      // EQ (Equal)
    execute = (registers.cpsr & (1 << 30)) != 0; // Z flag set
    break;
  case 0x1:                                      // NE (Not Equal)
    execute = (registers.cpsr & (1 << 30)) == 0; // Z flag clear
    break;
  case 0x2:                                      // CS/HS (Carry Set/Unsigned Higher or Same)
    execute = (registers.cpsr & (1 << 29)) != 0; // C flag set
    break;
  case 0x3:                                      // CC/LO (Carry Clear/Unsigned Lower)
    execute = (registers.cpsr & (1 << 29)) == 0; // C flag clear
    break;
  case 0x4:                                      // MI (Minus/Negative)
    execute = (registers.cpsr & (1 << 31)) != 0; // N flag set
    break;
  case 0x5:                                      // PL (Plus/Positive or Zero)
    execute = (registers.cpsr & (1 << 31)) == 0; // N flag clear
    break;
  case 0x6:                                      // VS (Overflow)
    execute = (registers.cpsr & (1 << 28)) != 0; // V flag set
    break;
  case 0x7:                                      // VC (No Overflow)
    execute = (registers.cpsr & (1 << 28)) == 0; // V flag clear
    break;
  case 0x8:                                                                               // HI (Unsigned Higher)
    execute = ((registers.cpsr & (1 << 29)) != 0) && ((registers.cpsr & (1 << 30)) == 0); // C flag set and Z flag clear
    break;
  case 0x9:                                                                               // LS (Unsigned Lower or Same)
    execute = ((registers.cpsr & (1 << 29)) == 0) || ((registers.cpsr & (1 << 30)) != 0); // C flag clear or Z flag set
    break;
  case 0xA:                                                                   // GE (Signed Greater than or Equal)
    execute = ((registers.cpsr & (1 << 31)) == (registers.cpsr & (1 << 28))); // N flag equals V flag
    break;
  case 0xB:                                                                   // LT (Signed Less than)
    execute = ((registers.cpsr & (1 << 31)) != (registers.cpsr & (1 << 28))); // N flag not equal to V flag
    break;
  case 0xC:                                                                                                          // GT (Signed Greater than)
    execute = ((registers.cpsr & (1 << 30)) == 0) && ((registers.cpsr & (1 << 31)) == (registers.cpsr & (1 << 28))); // Z flag clear and N flag equals V flag
    break;
  case 0xD:                                                                                                          // LE (Signed Less than or Equal)
    execute = ((registers.cpsr & (1 << 30)) != 0) || ((registers.cpsr & (1 << 31)) != (registers.cpsr & (1 << 28))); // Z flag set or N flag not equal to V flag
    break;
  case 0xE: // AL (Always)
    execute = true;
    break;
  case 0xF: // NV (Never, should not be used)
    execute = false;
    break;
  }

  if (!execute)
  {
    return; // Do not execute the instruction if the condition is not met
  }

  // Decoding the class of the instruction(bits 24 - 27)
  switch (instruction >> 24 & 0xF)
  {
  case 0x0: // ALU
  case 0x1: // ALU
  case 0x2: // ALU
  case 0x3: // ALU
  case 0x4: // ALU
  case 0x6: // ALU
  case 0x7: // ALU
  {
    executeALU(instruction);
    break;
  }
  case 0xA: // Branch
  {
    executeBranch(instruction);
    break;
  }
  case 0x5: // Load/Store
  {
    executeLoadStore(instruction);
    break;
  }
  default:
  {
    executeUndefined(instruction);
    break;
  }
  }
}

void CPU::executeALU(uint32_t instruction)
{
  // repeat parse bit 24
  uint32_t opcode = (instruction >> 21) & 0xF; // Extract bits 24-21
  switch (opcode)
  {
  case 0x0: // AND
  {
    uint32_t destReg = (instruction >> 12) & 0xF;
    uint32_t sourceReg = (instruction >> 16) & 0xF;
    uint32_t andReg = instruction & 0xFFF;
    writeRegister(destReg, readRegister(sourceReg) & andReg);
    if (instruction & (1 << 20))
      updateFlags(readRegister(sourceReg) & andReg, false, false); // AND doesn't affect Carry (C) or Overflow (V)
    break;
  }
  case 0x1: // EOR
  {
    uint32_t destReg = (instruction >> 12) & 0xF;
    uint32_t sourceReg = (instruction >> 16) & 0xF;
    uint32_t eorReg = instruction & 0xFFF;
    writeRegister(destReg, readRegister(sourceReg) ^ eorReg);
    if (instruction & (1 << 20))
      updateFlags(readRegister(sourceReg) ^ eorReg, false, false); // EOR doesn't affect Carry (C) or Overflow (V)
    break;
  }
  case 0x2: // SUB
  {
    uint32_t destReg = (instruction >> 12) & 0xF;
    uint32_t sourceReg = (instruction >> 16) & 0xF;
    uint32_t subVal = instruction & 0xFFF;

    uint32_t src = readRegister(sourceReg);
    uint32_t result = src - subVal;

    // Carry (C) is set if Rn >= Operand2 (no borrow)
    bool carry = (src >= subVal);

    // Overflow (V): Occurs if subtraction results in signed overflow
    bool overflow = ((src ^ subVal) & (src ^ result) & 0x80000000);

    writeRegister(destReg, result);

    // Update flags if S is set
    if (instruction & (1 << 20))
      updateFlags(result, carry, overflow);

    break;
  }
  case 0x3: // RSB
  {
    uint32_t destReg = (instruction >> 12) & 0xF;
    uint32_t sourceReg = (instruction >> 16) & 0xF;
    uint32_t rsbVal = instruction & 0xFFF;

    uint32_t src = readRegister(sourceReg);
    uint32_t result = rsbVal - src;

    // Carry (C) is set if Operand2 >= Rn (no borrow)
    bool carry = (rsbVal >= src);

    // Overflow (V): Occurs if subtraction results in signed overflow
    bool overflow = ((rsbVal ^ src) & (rsbVal ^ result) & 0x80000000);

    writeRegister(destReg, result);

    // Update flags if S is set
    if (instruction & (1 << 20))
      updateFlags(result, carry, overflow);

    break;
  }
  case 0x4: // ADD (ARM mode)
  {
    // Extract registers and operands
    uint32_t destReg = (instruction >> 12) & 0xF;    // Destination register (Rd)
    uint32_t sourceReg1 = (instruction >> 16) & 0xF; // First source register (Rn)
    uint32_t operand2 = instruction & 0xFFF;         // Second operand (immediate or register)

    uint32_t src1 = readRegister(sourceReg1); // Read value of Rn
    uint32_t src2;

    // Check if operand2 is an immediate or a register
    if (instruction & (1 << 25)) // Bit 25: Immediate flag
    {
      // Immediate value: Extract and rotate
      uint32_t imm = operand2 & 0xFF;                            // Lower 8 bits
      uint32_t rotate = (operand2 >> 8) & 0xF;                   // Upper 4 bits specify rotation
      src2 = (imm >> (2 * rotate)) | (imm << (32 - 2 * rotate)); // Rotate right
    }
    else
    {
      // Second operand is a register
      uint32_t sourceReg2 = operand2 & 0xF; // Second source register (Rm)
      src2 = readRegister(sourceReg2);      // Read value of Rm

      // Handle shift operation if specified
      uint32_t shiftType = (instruction >> 5) & 0x3;    // Bits 5–6: shift type
      uint32_t shiftAmount = (instruction >> 7) & 0x1F; // Bits 7–11: shift amount

      switch (shiftType)
      {
      case 0: // LSL (Logical Shift Left)
        src2 <<= shiftAmount;
        break;
      case 1: // LSR (Logical Shift Right)
        src2 = (shiftAmount == 0) ? 0 : (src2 >> shiftAmount);
        break;
      case 2: // ASR (Arithmetic Shift Right)
        src2 = (shiftAmount == 0) ? (src2 >> 31) : ((int32_t)src2 >> shiftAmount);
        break;
      case 3: // ROR (Rotate Right)
        if (shiftAmount == 0)
        {
          // RRX (Rotate Right with Extend)
          src2 = (registers.cpsr & (1 << 29)) ? (src2 >> 1) | 0x80000000 : (src2 >> 1);
        }
        else
        {
          src2 = (src2 >> shiftAmount) | (src2 << (32 - shiftAmount));
        }
        break;
      }
    }

    // Perform addition
    uint32_t result = src1 + src2;

    // Calculate flags
    bool carry = (result < src1);                                         // Unsigned overflow (carry)
    bool overflow = ((src1 ^ ~src2) & (src1 ^ result) & 0x80000000) != 0; // Signed overflow

    // Write result to destination register
    writeRegister(destReg, result);

    // Update flags if S bit is set
    if (instruction & (1 << 20)) // Bit 20: S flag
    {
      updateFlags(result, carry, overflow);
    }

    break;
  }
  case 0x5: // ADC
  {
    // Extract registers and operands
    uint32_t destReg = (instruction >> 12) & 0xF;    // Destination register (Rd)
    uint32_t sourceReg1 = (instruction >> 16) & 0xF; // First source register (Rn)
    uint32_t operand2 = instruction & 0xFFF;         // Second operand (immediate or register)

    uint32_t src1 = readRegister(sourceReg1); // Read value of Rn
    uint32_t src2;

    if (instruction & (1 << 25)) // Bit 25: Immediate flag
    {
      uint32_t imm = operand2 & 0xFF;                            // Lower 8 bits
      uint32_t rotate = (operand2 >> 8) & 0xF;                   // Upper 4 bits specify rotation
      src2 = (imm >> (2 * rotate)) | (imm << (32 - 2 * rotate)); // Rotate right
    }
    else
    {
      // Second operand is a register
      uint32_t sourceReg2 = operand2 & 0xF; // Second source register (Rm)
      src2 = readRegister(sourceReg2);      // Read value of Rm

      // Handle shift operation if specified
      uint32_t shiftType = (instruction >> 5) & 0x3;    // Bits 5–6: shift type
      uint32_t shiftAmount = (instruction >> 7) & 0x1F; // Bits 7–11: shift amount

      switch (shiftType)
      {
      case 0: // LSL (Logical Shift Left)
        src2 <<= shiftAmount;
        break;
      case 1: // LSR (Logical Shift Right)
        src2 = (shiftAmount == 0) ? 0 : (src2 >> shiftAmount);
        break;
      case 2: // ASR (Arithmetic Shift Right)
        src2 = (shiftAmount == 0) ? (src2 >> 31) : ((int32_t)src2 >> shiftAmount);
        break;
      case 3: // ROR (Rotate Right)
        if (shiftAmount == 0)
        {
          // RRX (Rotate Right with Extend)
          src2 = (registers.cpsr & (1 << 29)) ? (src2 >> 1) | 0x80000000 : (src2 >> 1);
        }
        else
        {
          src2 = (src2 >> shiftAmount) | (src2 << (32 - shiftAmount));
        }
        break;
      }
    }

    uint32_t result = src1 + src2 + ((registers.cpsr >> 29) & 1); // Add Carry (C) flag

    bool carry = (result < src1); // Unsigned overflow (carry)

    bool overflow = ((src1 ^ ~src2) & (src1 ^ result) & 0x80000000) != 0; // Signed overflow

    writeRegister(destReg, result);

    if (instruction & (1 << 20))
      updateFlags(result, carry, overflow);

    break;
  }
  case 0xA: // CMP
  {
    uint32_t sourceReg = (instruction >> 16) & 0xF;
    uint32_t cmpVal = instruction & 0xFFF;

    uint32_t src = readRegister(sourceReg);
    uint32_t result = src - cmpVal;

    bool carry = (src >= cmpVal);
    bool overflow = ((src ^ cmpVal) & (src ^ result) & 0x80000000);

    updateFlags(result, carry, overflow);

    break;
  }

  case 0xD: // MOV
  {
    uint32_t reg = (instruction >> 12) & 0xF;
    uint32_t imm = instruction & 0xFFF;
    writeRegister(reg, imm);

    // Update flags only if S bit is set
    if (instruction & (1 << 20))
      updateFlags(imm, false, false); // MOV doesn't affect Carry (C) or Overflow (V)

    break;
  }
  default:
  {
    executeUndefined(instruction);
    break;
  }
  }
}

void CPU::executeBranch(uint32_t instruction)
{
  int32_t offset = (instruction & 0xFFFFFF) << 2; // Extract 24-bit offset, multiply by 4
  offset = (offset << 6) >> 6;                    // Sign-extend the 26-bit offset
  registers.pc += offset + 4;
}

void CPU::executeLoadStore(uint32_t instruction)
{
  uint32_t opcode = (instruction >> 20) & 0xFF; // Extract bits 27-20
  uint32_t baseReg = (instruction >> 16) & 0xF; // Base register (Rn)
  uint32_t destReg = (instruction >> 12) & 0xF; // Destination/source register (Rd)
  uint32_t offset = instruction & 0xFFF;        // Immediate offset

  uint32_t address = readRegister(baseReg) + offset;

  switch (opcode & 0x0F)
  {
  case 0x09: // LDR (0x59)
  {
    uint32_t value = memory.readWord(address);
    writeRegister(destReg, value);
    break;
  }
  case 0x08: // STR (0x58)
  {
    uint32_t value = readRegister(destReg);
    memory.writeWord(address, value);
    break;
  }
  default:
  {
    std::cerr << "Unknown LDR/STR variant: 0x" << std::hex << instruction << std::endl;
    break;
  }
  }
}

void CPU::executeUndefined(uint32_t instruction)
{
  std::cerr << "Unknown ARM instruction: 0x" << std::hex << instruction << std::endl;
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
    uint32_t instruction = memory.readWord(registers.pc);
    registers.pc += 4;
    decodeARM(instruction);
  }
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
    std::cout << "CPSR: " << std::hex << registers.cpsr << std::endl;
    std::cout << "N (Negative): " << ((registers.cpsr >> 31) & 1) << std::endl;
    std::cout << "Z (Zero): " << ((registers.cpsr >> 30) & 1) << std::endl;
    std::cout << "C (Carry): " << ((registers.cpsr >> 29) & 1) << std::endl;
    std::cout << "V (Overflow): " << ((registers.cpsr >> 28) & 1) << std::endl;
    std::cout << "T (Thumb mode): " << ((registers.cpsr >> 5) & 1) << std::endl;
  }
  std::cout << "\n\n----Reached END----\n\n";
}