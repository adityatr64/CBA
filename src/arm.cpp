#include <iostream>

#include "../headers/cpu.hpp"
#include "../headers/memory.hpp"

/*
I've split this to keep too much code accumulation in one file
This file will contain the implementation of the ARM CPU class methods.
*/

// This decode arm need to be fixed

bool CPU::checkCondition(uint32_t inst) {
  uint32_t condition = (inst >> 28) & 0xF;
  bool execute = false;
  switch (condition) {
    case 0x0:                                       // EQ (Equal)
      execute = (registers.cpsr & (1 << 30)) != 0;  // Z flag set
      break;
    case 0x1:                                       // NE (Not Equal)
      execute = (registers.cpsr & (1 << 30)) == 0;  // Z flag clear
      break;
    case 0x2:                                       // CS/HS (Carry Set/Unsigned Higher or Same)
      execute = (registers.cpsr & (1 << 29)) != 0;  // C flag set
      break;
    case 0x3:                                       // CC/LO (Carry Clear/Unsigned Lower)
      execute = (registers.cpsr & (1 << 29)) == 0;  // C flag clear
      break;
    case 0x4:                                       // MI (Minus/Negative)
      execute = (registers.cpsr & (1 << 31)) != 0;  // N flag set
      break;
    case 0x5:                                       // PL (Plus/Positive or Zero)
      execute = (registers.cpsr & (1 << 31)) == 0;  // N flag clear
      break;
    case 0x6:                                       // VS (Overflow)
      execute = (registers.cpsr & (1 << 28)) != 0;  // V flag set
      break;
    case 0x7:                                       // VC (No Overflow)
      execute = (registers.cpsr & (1 << 28)) == 0;  // V flag clear
      break;
    case 0x8:  // HI (Unsigned Higher)
      execute = ((registers.cpsr & (1 << 29)) != 0) &&
                ((registers.cpsr & (1 << 30)) == 0);  // C flag set and Z flag clear
      break;
    case 0x9:  // LS (Unsigned Lower or Same)
      execute = ((registers.cpsr & (1 << 29)) == 0) ||
                ((registers.cpsr & (1 << 30)) != 0);  // C flag clear or Z flag set
      break;
    case 0xA:  // GE (Signed Greater than or Equal)
      execute =
          ((registers.cpsr & (1 << 31)) == (registers.cpsr & (1 << 28)));  // N flag equals V flag
      break;
    case 0xB:  // LT (Signed Less than)
      execute = ((registers.cpsr & (1 << 31)) !=
                 (registers.cpsr & (1 << 28)));  // N flag not equal to V flag
      break;
    case 0xC:  // GT (Signed Greater than)
      execute = ((registers.cpsr & (1 << 30)) == 0) &&
                ((registers.cpsr & (1 << 31)) ==
                 (registers.cpsr & (1 << 28)));  // Z flag clear and N flag equals V flag
      break;
    case 0xD:  // LE (Signed Less than or Equal)
      execute = ((registers.cpsr & (1 << 30)) != 0) ||
                ((registers.cpsr & (1 << 31)) !=
                 (registers.cpsr & (1 << 28)));  // Z flag set or N flag not equal to V flag
      break;
    case 0xE:  // AL (Always)
      execute = true;
      break;
    case 0xF:  // NV (Never, should not be used)
      execute = false;
      break;
  }
  return execute;
}

void CPU::decodeARM(uint32_t inst) {
  std::cout << "ARM inst: 0x" << std::hex << inst << std::endl;
  if (!checkCondition(inst) || inst == 0) {
    return;
  }

  uint32_t instClass = (inst >> 26) & 0b11;

  switch (instClass) {
    case 0b00:  // Data Processing
    {
      if (((inst & 0xFFFFFFF) >> 4) == 0x012FFF1)  // Branch exchange
      {
        std::cout << "Branch and Exchange" << std::endl;
        // executeArmBranchAndExchange();
        break;
      } else if (((inst >> 4) & 0xFF) == 0b00001001 &&
                 ((inst >> 24) & 0x1) == 0b1)  // SWP (bits 11-4 == 1001, bits 24 == 1)
      {
        std::cout << "Swap" << std::endl;
        executeArmSWP(inst);
        break;
      } else if (((inst >> 4) & 0xF) == 0b1001)  // MUL/MLA (bits 7-4 == 1001)
      {
        executeArmMultiply(inst);
        break;
      } else if (((inst >> 4) & 0xF) == 0b1011)  // Single Data Transfer or Half-word transfer
      {
        if (((inst >> 4) & 0xF) == 0xD && ((inst >> 20) & 0xF) == 0x0) {
          std::cout << "Load Double Word" << std::endl;
          // executeArmLDRD(inst);
        } else if (((inst >> 4) & 0xF) == 0xF && ((inst >> 20) & 0xF) == 0x0) {
          std::cout << "Store Double Word" << std::endl;
          // executeArmSTRD(inst);
        } else if (((inst >> 5) & 0x3) == 0x1)  // Half-word and signed transfers
        {
          std::cout << "Half-word/Signed Data Transfer" << std::endl;
          // executeArmHalfWordTransfer(inst);
        } else  // Regular ARM ALU operation
        {
          std::cout << "Data Processing" << std::endl;
          executeArmALU(inst);
        }
        break;
      } else  // Regular ARM ALU operation
      {
        std::cout << "Data Processing" << std::endl;
        executeArmALU(inst);
        break;
      }
      break;
    }
    case 0b01:  // Load/Store
    {
      executeArmLoadStore(inst);
      break;
    }
    case 0b10:  // Block Data Transfer
    {
      // executeArmBlockTransfer(inst);
      break;
    }
    case 0b11:  // SWI/co processor
    {
      std::cout << ((inst >> 24) & 0xF) << std::endl;
      if (((inst >> 24) & 0xF) == 0xF) executeArmSoftwareInterrupt(inst);
      // {
      //   executeArmBranchLink(inst);
      // }
      // else
      // {
      //   executeArmBranch(inst);
      // }
      break;
    }
    default: {
      executeArmUndefined(inst);
      break;
    }
  }
}


// Decode and execute ARM insts

void CPU::executeArmBranchLink(uint32_t inst) {
  int32_t offset = (inst & 0xFFFFFF) << 2;  // Extract 24-bit offset, multiply by 4
  offset = (offset << 6) >> 6;              // Sign-extend the 26-bit offset
  writeRegister(14, registers.pc - 4);      // Save return address in LR (R14)
  registers.pc += offset + 4;
}

void CPU::executeArmBranch(uint32_t inst) {
  int32_t offset = (inst & 0xFFFFFF) << 2;  // Extract 24-bit offset, multiply by 4
  offset = (offset << 6) >> 6;              // Sign-extend the 26-bit offset
  registers.pc += offset + 4;
}

void CPU::executeArmBlockTransfer(uint32_t inst) {
  // Implementation of the ARM block transfer inst
  std::cerr << "ARM Block Transfer inst: 0x" << std::hex << inst << std::endl;
}

void CPU::executeArmSoftwareInterrupt(uint32_t inst) {
  std::cerr << "Software Interrupt: 0x" << std::hex << inst << std::endl;
  exit(0);
}

void CPU::executeArmLoadStore(uint32_t inst) {
  uint32_t opcode = (inst >> 20) & 0x1;   // Extract bits 27-20
  uint32_t baseReg = (inst >> 16) & 0xF;  // Base register (Rn)
  uint32_t destReg = (inst >> 12) & 0xF;  // Destination/source register (Rd)
  uint32_t offset = inst & 0xFFF;         // Immediate offset
  bool byte = (inst >> 22) & 1;

  if (baseReg == 15) {
    offset += 4;
  }
  uint32_t address = readRegister(baseReg) + offset;

  switch (opcode) {
    case 0x1:  // LDR (0x59)
    {
      if (byte) {
        uint8_t value = memory.readByte(address);
        writeRegister(destReg, value);
      } else {
        uint32_t value = memory.readWord(address);
        writeRegister(destReg, value);
      }
      break;
    }
    case 0x0:  // STR (0x58)
    {
      if (byte) {
        uint8_t value = readRegister(destReg);
        memory.writeByte(address, value);
      } else {
        uint32_t value = readRegister(destReg);
        memory.writeWord(address, value);
      }
      break;
    }
    default: {
      std::cerr << "Unknown LDR/STR variant: 0x" << std::hex << inst << std::endl;
      break;
    }
  }
}

void CPU::executeArmUndefined(uint32_t inst) {
  std::cerr << "Unknown ARM inst: 0x" << std::hex << inst << std::endl;
}

void CPU::executeArmSWP(uint32_t inst) {
  bool Byte = (inst >> 22) & 1;
  uint32_t Rn = (inst >> 16) & 0xF;
  uint32_t Rd = (inst >> 12) & 0xF;
  uint32_t Rm = (inst & 0xF);

  uint32_t address = readRegister(Rn);
  uint32_t value = readRegister(Rm);

  if (Byte) {
    uint8_t data = memory.readByte(address);
    memory.writeByte(address, value & 0xFF);
    writeRegister(Rd, data);
  } else {
    uint32_t data = memory.readWord(address);
    memory.writeWord(address, value);
    writeRegister(Rd, data);
  }
}

void CPU::executeArmMultiply(uint32_t inst) {
  bool accumulate = (inst >> 21) & 0x1;

  uint32_t Rd = (inst >> 16) & 0xF;
  uint32_t Rn = (inst >> 12) & 0xF;
  uint32_t Rs = (inst >> 8) & 0xF;
  uint32_t Rm = (inst & 0xF);

  uint32_t operand1 = readRegister(Rm);
  uint32_t operand2 = readRegister(Rs);
  uint32_t result;

  if (accumulate) {
    result = readRegister(Rn) + (operand1 * operand2);
    std::cout << "MLA: R" << Rd << " = R" << Rn << " + (R" << Rm << " * R" << Rs << ")"
              << std::endl;
  } else {
    result = operand1 * operand2;
    std::cout << "MUL: R" << Rd << " = R" << Rm << " * R" << Rs << std::endl;
  }

  writeRegister(Rd, result);
  updateFlags(result, false, false);  // As before, MUL/MLA don’t set C or V

  std::cout << "Result: " << std::hex << result << std::endl;
  std::cout << "CPSR: " << std::hex << registers.cpsr << std::endl;
}

void CPU::executeArmALU(uint32_t inst) {
  bool carryout = false, overflow = false;
  uint32_t opcode = (inst >> 21) & 0xF;  // Extract bits 24-21
  uint32_t destReg = (inst >> 12) & 0xF;
  uint32_t sourceReg = (inst >> 16) & 0xF;
  uint32_t operand2 = selectOperand2(inst, carryout);
  uint32_t src = readRegister(sourceReg);
  uint32_t result = 0;

  switch (opcode) {
    case 0x0:  // AND
    {
      result = src & operand2;
      break;
    }
    case 0x1:  // EOR
    {
      result = src ^ operand2;
      break;
    }
    case 0x2:  // SUB
    {
      result = src - operand2;
      //
      carryout = (src >= operand2);
      // Overflow (V): Occurs if subtraction results in signed overflow
      overflow = ((src ^ operand2) & (src ^ result) & 0x80000000);
      break;
    }
    case 0x3:  // RSB
    {
      result = operand2 - src;
      // Carry (C) is set if Operand2 >= Rn (no borrow)
      carryout = (operand2 >= src);
      // Overflow (V): Occurs if subtraction results in signed overflow
      overflow = ((operand2 ^ src) & (operand2 ^ result) & 0x80000000);
      break;
    }
    case 0x4:  // ADD (ARM mode)
    {
      // Perform addition
      result = src + operand2;

      // Calculate flags
      carryout = (result < src);  // Unsigned overflow (carry)
      overflow = ((src ^ ~operand2) & (src ^ result) & 0x80000000) != 0;  // Signed overflow

      break;
    }
    case 0x5:  // ADC
    {
      result = src + operand2 + ((registers.cpsr >> 29) & 1);  // Add Carry (C) flag
      carryout = (result < src);                               // Unsigned overflow (carry)
      overflow = ((src ^ ~operand2) & (src ^ result) & 0x80000000) != 0;  // Signed overflow
      break;
    }
    case 0x6:  // SBC
    {
      result = src - operand2 - (carryout ? 0 : 1);  // Subtract Carry (C) flag
      carryout = (src >= operand2);                  // Unsigned overflow (carry)
      overflow = ((src ^ operand2) & (src ^ result) & 0x80000000) != 0;  // Signed overflow
      break;
    }
    case 0x7:  // RSC
    {
      result = operand2 - src - (carryout ? 0 : 1);  // Reverse Subtract Carry (C) flag
      carryout = (operand2 >= src);                  // Unsigned overflow (carry)
      overflow = ((operand2 ^ src) & (operand2 ^ result) & 0x80000000) != 0;  // Signed overflow
      break;
    }
    case 0x8:  // TST
    {
      result = src & operand2;
      break;
    }
    case 0x9:  // TEQ
    {
      result = src ^ operand2;
      break;
    }
    case 0xA:  // CMP
    {
      result = src - operand2;
      carryout = (src >= operand2);
      overflow = ((src ^ operand2) & (src ^ result) & 0x80000000);

      updateFlags(result, carryout, overflow);
      return;
    }
    case 0xB:  // CMN
    {
      result = src + operand2;
      carryout = (result < src);
      overflow = ((src ^ ~operand2) & (src ^ result) & 0x80000000) != 0;

      updateFlags(result, carryout, overflow);
      return;
    }
    case 0xC:  // ORR
    {
      result = src | operand2;
      break;
    }
    case 0xD:  // MOV
    {
      result = operand2;
      break;
    }
    case 0xE:  // BIC
    {
      result = src & ~operand2;
      break;
    }
    case 0xF:  // MVN
    {
      result = ~operand2;
      break;
    }
    default: {
      executeArmUndefined(inst);
      break;
    }
  }
  writeRegister(destReg, result);
  if (inst & (1 << 20)) updateFlags(result, carryout, overflow);
}

uint32_t CPU::Shifter(uint32_t value, uint32_t type, uint32_t amount, bool& carryout) {
  switch (type) {
    case 0:  // LSL (Logical Shift Left)
      carryout = (amount > 0) ? (value & (1 << (32 - amount))) : false;
      value <<= amount;
      break;
    case 1:  // LSR (Logical Shift Right)
      carryout = (amount > 0) ? (value & (1 << (amount - 1))) : false;
      value = (amount == 0) ? 0 : (value >> amount);
      break;
    case 2:  // ASR (Arithmetic Shift Right)
      carryout = (amount > 0) ? (value & (1 << (amount - 1))) : false;
      value = (amount == 0) ? (value >> 31) : ((int32_t)value >> amount);
      break;
    case 3:  // ROR (Rotate Right)
      if (amount == 0) {
        // RRX (Rotate Right with Extend)

        carryout = (value & 1);
        value = (registers.cpsr & (1 << 29)) ? (value >> 1) | 0x80000000 : (value >> 1);
      } else {
        carryout = (amount > 0) ? (value & (1 << (amount - 1))) : false;
        value = (value >> amount) | (value << (32 - amount));
      }
      break;
    default:
      return value;
  }
  return value;
}

uint32_t CPU::selectOperand2(uint32_t inst, bool& carryOut) {
  uint32_t operand2 = inst & 0xFFF;
  carryOut = false;
  if (inst & (1 << 25)) {
    uint32_t imm = operand2 & 0xFF;                             // Lower 8 bits
    uint32_t rotate = (operand2 >> 8) & 0xF;                    // Upper 4 bits specify rotation
    return (imm >> (2 * rotate)) | (imm << (32 - 2 * rotate));  // Rotate right
  } else {
    uint32_t rm = operand2 & 0xF;
    uint32_t shiftType = (inst >> 5) & 0x3;
    uint32_t shiftAmount = (inst >> 7) & 0x1F;

    return Shifter(readRegister(rm), shiftType, shiftAmount, carryOut);
  }
}
