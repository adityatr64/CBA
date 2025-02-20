#include "cpu.hpp"
#include <iostream>
#include "memory.hpp"
/*
I've split this to keep too much code accumulation in one file
This file will contain the implementation of the ARM CPU class methods.
*/

void CPU::decodeARM(uint32_t instruction)
{
  std::cout << "ARM Instruction: 0x" << std::hex << instruction << std::endl;

  uint32_t condition = (instruction >> 28) & 0xF; // Extract condition field (bits 31-28)
  bool execute = false;

  // Condition check [COND] part of the instruction
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
  case 0x0:
  case 0x1:
  case 0x2:
  case 0x3:
  case 0x4:
  case 0x6:
  case 0x7:
  {
    executeArmALU(instruction);
    break; // ALU Instructions
  }
  case 0x5:
  {
    executeArmLoadStore(instruction);
    break; // Load/Store Instructions
  }
  case 0x8:
  {
    executeArmBlockTransfer(instruction);
    break; // STM (Store Multiple Registers)
  }
  case 0x9:
  {
    executeArmBlockTransfer(instruction);
    break; // LDM (Load Multiple Registers)
  }
  case 0xA:
  {
    executeArmBranch(instruction);
    break; // B (Branch)
  }
  case 0xB:
  {
    executeArmBranchLink(instruction);
    break; // BL (Branch with Link)
  }
  case 0xC:
  case 0xD:
  {
    // executeArmCoprocessorLoadStore(instruction);
    break; // Coprocessor Load/Store
  }
  case 0xE:
  {
    // executeArmCoprocessorDataProcessing(instruction);
    break; // Coprocessor Data Processing
  }
  case 0xF:
  {
    executeArmSoftwareInterrupt(instruction);
    break; // SWI (Software Interrupt)
  }
  default:
  {

    executeArmUndefined(instruction);
    break; // Undefined Instructions
  }
  }
}

uint32_t CPU::Shifter(uint32_t value, uint32_t type, uint32_t amount, bool &carryout)
{
  switch (type)
  {
  case 0: // LSL (Logical Shift Left)
    carryout = (amount > 0) ? (value & (1 << (32 - amount))) : false;
    value <<= amount;
    break;
  case 1: // LSR (Logical Shift Right)
    carryout = (amount > 0) ? (value & (1 << (amount - 1))) : false;
    value = (amount == 0) ? 0 : (value >> amount);
    break;
  case 2: // ASR (Arithmetic Shift Right)
    carryout = (amount > 0) ? (value & (1 << (amount - 1))) : false;
    value = (amount == 0) ? (value >> 31) : ((int32_t)value >> amount);
    break;
  case 3: // ROR (Rotate Right)
    if (amount == 0)
    {
      // RRX (Rotate Right with Extend)

      carryout = (value & 1);
      value = (registers.cpsr & (1 << 29)) ? (value >> 1) | 0x80000000 : (value >> 1);
    }
    else
    {
      carryout = (amount > 0) ? (value & (1 << (amount - 1))) : false;
      value = (value >> amount) | (value << (32 - amount));
    }
    break;
  default:
    return value;
  }
  return value;
}

uint32_t CPU::selectOperand2(uint32_t instruction, bool &carryOut)
{
  uint32_t operand2 = instruction & 0xFFF;
  carryOut = false;
  if (instruction & (1 << 25))
  {
    uint32_t imm = operand2 & 0xFF;                            // Lower 8 bits
    uint32_t rotate = (operand2 >> 8) & 0xF;                   // Upper 4 bits specify rotation
    return (imm >> (2 * rotate)) | (imm << (32 - 2 * rotate)); // Rotate right
  }
  else
  {
    uint32_t rm = operand2 & 0xF;
    uint32_t shiftType = (instruction >> 5) & 0x3;
    uint32_t shiftAmount = (instruction >> 7) & 0x1F;

    return Shifter(readRegister(rm), shiftType, shiftAmount, carryOut);
  }
}

// Decode and execute ARM instructions
void CPU::executeArmALU(uint32_t instruction)
{
  // repeat parse bit 24
  bool carryout = false, overflow = false;
  uint32_t opcode = (instruction >> 21) & 0xF; // Extract bits 24-21
  uint32_t destReg = (instruction >> 12) & 0xF;
  uint32_t sourceReg = (instruction >> 16) & 0xF;
  uint32_t operand2 = selectOperand2(instruction, carryout);
  uint32_t src = readRegister(sourceReg);
  uint32_t result = 0;

  switch (opcode)
  {
  case 0x0: // AND
  {
    result = src & operand2;
    break;
  }
  case 0x1: // EOR
  {
    result = src ^ operand2;
    break;
  }
  case 0x2: // SUB
  {
    result = src - operand2;
    //
    carryout = (src >= operand2);
    // Overflow (V): Occurs if subtraction results in signed overflow
    overflow = ((src ^ operand2) & (src ^ result) & 0x80000000);
    break;
  }
  case 0x3: // RSB
  {
    result = operand2 - src;
    // Carry (C) is set if Operand2 >= Rn (no borrow)
    carryout = (operand2 >= src);
    // Overflow (V): Occurs if subtraction results in signed overflow
    overflow = ((operand2 ^ src) & (operand2 ^ result) & 0x80000000);
    break;
  }
  case 0x4: // ADD (ARM mode)
  {
    // Perform addition
    result = src + operand2;

    // Calculate flags
    carryout = (result < src);                                         // Unsigned overflow (carry)
    overflow = ((src ^ ~operand2) & (src ^ result) & 0x80000000) != 0; // Signed overflow

    break;
  }
  case 0x5: // ADC
  {
    result = src + operand2 + ((registers.cpsr >> 29) & 1);            // Add Carry (C) flag
    carryout = (result < src);                                         // Unsigned overflow (carry)
    overflow = ((src ^ ~operand2) & (src ^ result) & 0x80000000) != 0; // Signed overflow
    break;
  }
  case 0x6: // SBC
  {
    result = src - operand2 - (carryout ? 0 : 1);                     // Subtract Carry (C) flag
    carryout = (src >= operand2);                                     // Unsigned overflow (carry)
    overflow = ((src ^ operand2) & (src ^ result) & 0x80000000) != 0; // Signed overflow
    break;
  }
  case 0x7: // RSC
  {
    result = operand2 - src - (carryout ? 0 : 1);                          // Reverse Subtract Carry (C) flag
    carryout = (operand2 >= src);                                          // Unsigned overflow (carry)
    overflow = ((operand2 ^ src) & (operand2 ^ result) & 0x80000000) != 0; // Signed overflow
    break;
  }
  case 0x8: // TST
  {
    result = src & operand2;
    break;
  }
  case 0x9: // TEQ
  {
    result = src ^ operand2;
    break;
  }
  case 0xA: // CMP
  {
    result = src - operand2;
    carryout = (src >= operand2);
    overflow = ((src ^ operand2) & (src ^ result) & 0x80000000);

    updateFlags(result, carryout, overflow);
    return;
  }
  case 0xB: // CMN
  {
    result = src + operand2;
    carryout = (result < src);
    overflow = ((src ^ ~operand2) & (src ^ result) & 0x80000000) != 0;

    updateFlags(result, carryout, overflow);
    return;
  }
  case 0xC: // ORR
  {
    result = src | operand2;
    break;
  }
  case 0xD: // MOV
  {
    result = operand2;
    break;
  }
  case 0xE: // BIC
  {
    result = src & ~operand2;
    break;
  }
  case 0xF: // MVN
  {
    result = ~operand2;
    break;
  }
  default:
  {
    executeArmUndefined(instruction);
    break;
  }
  }
  writeRegister(destReg, result);
  if (instruction & (1 << 20))
    updateFlags(result, carryout, overflow);
}

void CPU::executeArmBranchLink(uint32_t instruction)
{
  // int32_t offset = (instruction & 0xFFFFFF) << 2; // Extract 24-bit offset, multiply by 4
  // offset = (offset << 6) >> 6;                    // Sign-extend the 26-bit offset
  // writeRegister(14, registers.pc - 4);            // Save return address in LR (R14)
  // registers.pc += offset + 4;
}

void CPU::executeArmBranch(uint32_t instruction)
{
  int32_t offset = (instruction & 0xFFFFFF) << 2; // Extract 24-bit offset, multiply by 4
  offset = (offset << 6) >> 6;                    // Sign-extend the 26-bit offset
  registers.pc += offset + 4;
}

void CPU::executeArmLoadStore(uint32_t instruction)
{
  uint32_t opcode = (instruction >> 20) & 0xFF; // Extract bits 27-20
  uint32_t baseReg = (instruction >> 16) & 0xF; // Base register (Rn)
  uint32_t destReg = (instruction >> 12) & 0xF; // Destination/source register (Rd)
  uint32_t offset = instruction & 0xFFF;        // Immediate offset

  uint32_t address = readRegister(baseReg) + offset + 4;

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

void CPU::executeArmBlockTransfer(uint32_t instruction)
{
  // Implementation of the ARM block transfer instruction
  std::cerr << "ARM Block Transfer instruction: 0x" << std::hex << instruction << std::endl;
}

void CPU::executeArmSoftwareInterrupt(uint32_t instruction)
{
  std::cerr << "Software Interrupt: 0x" << std::hex << instruction << std::endl;
}

void CPU::executeArmUndefined(uint32_t instruction)
{
  std::cerr << "Unknown ARM instruction: 0x" << std::hex << instruction << std::endl;
}
