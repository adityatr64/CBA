#include "../include/arm.hpp"

#include <iostream>

#include "../include/cpu.hpp"
#include "../include/memory.hpp"
#include "arm.hpp"

namespace ARM {

// Wrappers to match InstructionHandler type
void wrappedExecuteArmBranch(CPU* cpu, Memory* memory, uint32_t inst) {
  executeArmBranch(cpu, inst);
}

void wrappedExecuteArmBranchLink(CPU* cpu, Memory* memory, uint32_t inst) {
  executeArmBranchLink(cpu, inst);
}

void wrappedExecuteArmALU(CPU* cpu, Memory* memory, uint32_t inst) {
  executeArmALU(cpu, inst);
}

void wrappedExecuteArmMultiply(CPU* cpu, Memory* memory, uint32_t inst) {
  executeArmMultiply(cpu, inst);
}

void wrappedExecuteArmUndefined(CPU* cpu, Memory* memory, uint32_t inst) {
  executeArmUndefined(inst);
}

void wrappedExecuteArmSWI(CPU* cpu, Memory* memory, uint32_t inst) {
  executeArmSoftwareInterrupt(inst);
}

static const InstructionEntry armDispatchTable[] = {
    {0x0FFFFFF0, 0x012FFF10, wrappedExecuteArmBranch, "BX"},
    {0x0FBF0FFF, 0x010F0000, executeArmMRS, "MRS"},
    {0x0FBFF000, 0x0129F000, executeArmMRSregister, "MRS(Register)"},
    {0x0DBFF000, 0x0320F000, executeArmMRSimm, "MRS(Immediate)"},
    {0x0FF00FF0, 0x01000090, executeArmSWP, "SWP"},
    {0x0FC000F0, 0x00000090, wrappedExecuteArmMultiply, "MUL/MLA"},
    {0x0F8000F0, 0x00800090, executeArmMultiplyLong, "Multiply Long"},
    {0x0E4000F0, 0x004000B0, executeArmHalfWord, "Halfword/Sign Transfer"},

    {0x0E000000, 0x0A000000, wrappedExecuteArmBranch, "Branch"},
    {0x0E000000, 0x0B000000, wrappedExecuteArmBranchLink, "Branch with Link"},
    {0x0C000000, 0x04000000, executeArmLoadStore, "Load/Store"},
    {0x0C000000, 0x08000000, executeArmBlockTransfer, "Block Transfer"},
    {0x0F000000, 0x0F000000, wrappedExecuteArmSWI, "SWI"},
    {0x0C000000, 0x00000000, wrappedExecuteArmALU, "ALU"},

    // Fallback
    {0x00000000, 0x00000000, wrappedExecuteArmUndefined, "Undefined"}};
void decodeARM(CPU* cpu, Memory* memory, uint32_t inst) {
  std::cout << "ARM inst: 0x" << std::hex << inst << std::endl;
  if (!checkCondition(cpu, inst) || inst == 0) {
    return;
  }

  for (const auto& entry : armDispatchTable) {
    if ((inst & entry.mask) == entry.pattern) {
      std::cout << "Dispatch: " << entry.name << std::endl;
      entry.handler(cpu, memory, inst);
      return;
    }
  }

  wrappedExecuteArmUndefined(cpu, memory, inst);  // fallback
}

bool checkCondition(CPU* cpu, uint32_t inst) {
  uint32_t condition = (inst >> 28) & 0xF;
  uint32_t cpsr = cpu->getRegisters().cpsr;
  bool execute = false;

  switch (condition) {
    case 0x0:                             // EQ (Equal)
      execute = (cpsr & (1 << 30)) != 0;  // Z flag set
      break;
    case 0x1:                             // NE (Not Equal)
      execute = (cpsr & (1 << 30)) == 0;  // Z flag clear
      break;
    case 0x2:                             // CS/HS (Carry Set/Unsigned Higher or Same)
      execute = (cpsr & (1 << 29)) != 0;  // C flag set
      break;
    case 0x3:                             // CC/LO (Carry Clear/Unsigned Lower)
      execute = (cpsr & (1 << 29)) == 0;  // C flag clear
      break;
    case 0x4:                             // MI (Minus/Negative)
      execute = (cpsr & (1 << 31)) != 0;  // N flag set
      break;
    case 0x5:                             // PL (Plus/Positive or Zero)
      execute = (cpsr & (1 << 31)) == 0;  // N flag clear
      break;
    case 0x6:                             // VS (Overflow)
      execute = (cpsr & (1 << 28)) != 0;  // V flag set
      break;
    case 0x7:                             // VC (No Overflow)
      execute = (cpsr & (1 << 28)) == 0;  // V flag clear
      break;
    case 0x8:  // HI (Unsigned Higher)
      execute =
          ((cpsr & (1 << 29)) != 0) && ((cpsr & (1 << 30)) == 0);  // C flag set and Z flag clear
      break;
    case 0x9:  // LS (Unsigned Lower or Same)
      execute =
          ((cpsr & (1 << 29)) == 0) || ((cpsr & (1 << 30)) != 0);  // C flag clear or Z flag set
      break;
    case 0xA:                                                // GE (Signed Greater than or Equal)
      execute = ((cpsr & (1 << 31)) == (cpsr & (1 << 28)));  // N flag equals V flag
      break;
    case 0xB:                                                // LT (Signed Less than)
      execute = ((cpsr & (1 << 31)) != (cpsr & (1 << 28)));  // N flag not equal to V flag
      break;
    case 0xC:  // GT (Signed Greater than)
      execute =
          ((cpsr & (1 << 30)) == 0) &&
          ((cpsr & (1 << 31)) == (cpsr & (1 << 28)));  // Z flag clear and N flag equals V flag
      break;
    case 0xD:  // LE (Signed Less than or Equal)
      execute =
          ((cpsr & (1 << 30)) != 0) ||
          ((cpsr & (1 << 31)) != (cpsr & (1 << 28)));  // Z flag set or N flag not equal to V flag
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

void executeArmHalfWord(CPU* cpu, Memory* memory, uint32_t inst) {
  // placeholder
}

void executeArmMRS(CPU* cpu, Memory* memory, uint32_t inst) {
  // placeholder
}

void executeArmMRSregister(CPU* cpu, Memory* memory, uint32_t inst) {
  // placeholder
}

void executeArmMRSimm(CPU* cpu, Memory* memory, uint32_t inst) {
  // place holder
}

void executeArmMultiplyLong(CPU* cpu, Memory* memory, uint32_t inst) {
  // placeholder
}

void executeArmBranchLink(CPU* cpu, uint32_t inst) {
  int32_t offset = (inst & 0xFFFFFF) << 2;             // Extract 24-bit offset, multiply by 4
  offset = (offset << 6) >> 6;                         // Sign-extend the 26-bit offset
  cpu->writeRegister(14, cpu->getRegisters().pc - 4);  // Save return address in LR (R14)
  cpu->getRegisters().pc += offset + 4;
}

void executeArmBranch(CPU* cpu, uint32_t inst) {
  int32_t offset = (inst & 0xFFFFFF) << 2;  // Extract 24-bit offset, multiply by 4
  offset = (offset << 6) >> 6;              // Sign-extend the 26-bit offset
  cpu->getRegisters().pc += offset + 4;
}

void executeArmBlockTransfer(CPU* cpu, Memory* memory, uint32_t inst) {
  // Implementation of the ARM block transfer inst
  std::cerr << "ARM Block Transfer inst: 0x" << std::hex << inst << std::endl;
}

void executeArmSoftwareInterrupt(uint32_t inst) {
  std::cerr << "Software Interrupt: 0x" << std::hex << inst << std::endl;
  exit(0);
}

void executeArmLoadStore(CPU* cpu, Memory* memory, uint32_t inst) {
  uint32_t opcode = (inst >> 20) & 0x1;   // Extract bits 27-20
  uint32_t baseReg = (inst >> 16) & 0xF;  // Base register (Rn)
  uint32_t destReg = (inst >> 12) & 0xF;  // Destination/source register (Rd)
  uint32_t offset = inst & 0xFFF;         // Immediate offset
  bool byte = (inst >> 22) & 1;

  if (baseReg == 15) {
    offset += 4;
  }
  uint32_t address = cpu->readRegister(baseReg) + offset;

  switch (opcode) {
    case 0x1:  // LDR (0x59)
    {
      if (byte) {
        uint8_t value = memory->readByte(address);
        cpu->writeRegister(destReg, value);
      } else {
        uint32_t value = memory->readWord(address);
        cpu->writeRegister(destReg, value);
      }
      break;
    }
    case 0x0:  // STR (0x58)
    {
      if (byte) {
        uint8_t value = cpu->readRegister(destReg);
        memory->writeByte(address, value);
      } else {
        uint32_t value = cpu->readRegister(destReg);
        memory->writeWord(address, value);
      }
      break;
    }
    default: {
      std::cerr << "Unknown LDR/STR variant: 0x" << std::hex << inst << std::endl;
      break;
    }
  }
}

void executeArmUndefined(uint32_t inst) {
  std::cerr << "Unknown ARM inst: 0x" << std::hex << inst << std::endl;
}

void executeArmSWP(CPU* cpu, Memory* memory, uint32_t inst) {
  bool Byte = (inst >> 22) & 1;
  uint32_t Rn = (inst >> 16) & 0xF;
  uint32_t Rd = (inst >> 12) & 0xF;
  uint32_t Rm = (inst & 0xF);

  uint32_t address = cpu->readRegister(Rn);
  uint32_t value = cpu->readRegister(Rm);

  if (Byte) {
    uint8_t data = memory->readByte(address);
    memory->writeByte(address, value & 0xFF);
    cpu->writeRegister(Rd, data);
  } else {
    uint32_t data = memory->readWord(address);
    memory->writeWord(address, value);
    cpu->writeRegister(Rd, data);
  }
}

void executeArmMultiply(CPU* cpu, uint32_t inst) {
  bool accumulate = (inst >> 21) & 0x1;

  uint32_t Rd = (inst >> 16) & 0xF;
  uint32_t Rn = (inst >> 12) & 0xF;
  uint32_t Rs = (inst >> 8) & 0xF;
  uint32_t Rm = (inst & 0xF);

  uint32_t operand1 = cpu->readRegister(Rm);
  uint32_t operand2 = cpu->readRegister(Rs);
  uint32_t result;

  if (accumulate) {
    result = cpu->readRegister(Rn) + (operand1 * operand2);
    std::cout << "MLA: R" << Rd << " = R" << Rn << " + (R" << Rm << " * R" << Rs << ")"
              << std::endl;
  } else {
    result = operand1 * operand2;
    std::cout << "MUL: R" << Rd << " = R" << Rm << " * R" << Rs << std::endl;
  }

  cpu->writeRegister(Rd, result);
  updateFlags(cpu, result, false, false);  // MUL/MLA don't set C or V

  std::cout << "Result: " << std::hex << result << std::endl;
  std::cout << "CPSR: " << std::hex << cpu->getRegisters().cpsr << std::endl;
}

void executeArmALU(CPU* cpu, uint32_t inst) {
  bool carryout = false, overflow = false;
  uint32_t opcode = EXTRACT_BITS(inst, 21, 4);
  uint32_t destReg = EXTRACT_BITS(inst, 12, 4);
  uint32_t sourceReg = EXTRACT_BITS(inst, 16, 4);
  uint32_t operand2 = selectOperand2(cpu, inst, carryout);
  uint32_t src = cpu->readRegister(sourceReg);
  uint32_t result = 0;

  switch (opcode) {
    case 0x0:  // AND
      result = src & operand2;
      break;
    case 0x1:  // EOR
      result = src ^ operand2;
      break;
    case 0x2:  // SUB
      result = src - operand2;
      carryout = (src >= operand2);
      overflow = ((src ^ operand2) & (src ^ result) & 0x80000000);
      break;
    case 0x3:  // RSB
      result = operand2 - src;
      carryout = (operand2 >= src);
      overflow = ((operand2 ^ src) & (operand2 ^ result) & 0x80000000);
      break;
    case 0x4:  // ADD
      result = src + operand2;
      carryout = (result < src);
      overflow = ((src ^ ~operand2) & (src ^ result) & 0x80000000) != 0;
      break;
    case 0x5:  // ADC
      result = src + operand2 + ((cpu->getRegisters().cpsr >> 29) & 1);
      carryout = (result < src);
      overflow = ((src ^ ~operand2) & (src ^ result) & 0x80000000) != 0;
      break;
    case 0x6:  // SBC
      result = src - operand2 - (carryout ? 0 : 1);
      carryout = (src >= operand2);
      overflow = ((src ^ operand2) & (src ^ result) & 0x80000000) != 0;
      break;
    case 0x7:  // RSC
      result = operand2 - src - (carryout ? 0 : 1);
      carryout = (operand2 >= src);
      overflow = ((operand2 ^ src) & (operand2 ^ result) & 0x80000000) != 0;
      break;
    case 0x8:  // TST
      result = src & operand2;
      break;
    case 0x9:  // TEQ
      result = src ^ operand2;
      break;
    case 0xA:  // CMP
      result = src - operand2;
      carryout = (src >= operand2);
      overflow = ((src ^ operand2) & (src ^ result) & 0x80000000);
      updateFlags(cpu, result, carryout, overflow);
      return;
    case 0xB:  // CMN
      result = src + operand2;
      carryout = (result < src);
      overflow = ((src ^ ~operand2) & (src ^ result) & 0x80000000) != 0;
      updateFlags(cpu, result, carryout, overflow);
      return;
    case 0xC:  // ORR
      result = src | operand2;
      break;
    case 0xD:  // MOV
      result = operand2;
      break;
    case 0xE:  // BIC
      result = src & ~operand2;
      break;
    case 0xF:  // MVN
      result = ~operand2;
      break;
    default:
      executeArmUndefined(inst);
      return;
  }

  cpu->writeRegister(destReg, result);
  if (inst & (1 << 20)) updateFlags(cpu, result, carryout, overflow);
}

uint32_t Shifter(CPU* cpu, uint32_t value, uint32_t type, uint32_t amount, bool& carryout) {
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
        value = (cpu->getRegisters().cpsr & (1 << 29)) ? (value >> 1) | 0x80000000 : (value >> 1);
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

uint32_t selectOperand2(CPU* cpu, uint32_t inst, bool& carryOut) {
  uint32_t operand2 = EXTRACT_BITS(inst, 0, 12);
  carryOut = false;
  if (inst & (1 << 25)) {
    if (CHECK_BIT(inst, 25)) {
      uint32_t imm = EXTRACT_BITS(operand2, 0, 8);
      uint32_t rotate = EXTRACT_BITS(operand2, 8, 4);
      return (imm >> (2 * rotate)) | (imm << (32 - 2 * rotate));
    } else {
      uint32_t rm = EXTRACT_BITS(operand2, 0, 4);
      uint32_t shiftType = EXTRACT_BITS(inst, 5, 2);
      uint32_t shiftAmount = EXTRACT_BITS(inst, 7, 5);

      return Shifter(cpu, cpu->readRegister(rm), shiftType, shiftAmount, carryOut);
    }
  }
  return operand2;  // Default case
}

void updateFlags(CPU* cpu, uint32_t result, bool carry, bool overflow) {
  uint32_t& cpsr = cpu->getRegisters().cpsr;
  cpsr &= ~(0xF << 28);  // Clear N, Z, C, V flags (bits 28-31)
  if (result == 0)       // Zero Flag (Z)
    cpsr |= (1 << 30);
  if (result & 0x80000000)  // Negative Flag (N)
    cpsr |= (1 << 31);
  if (carry)  // Carry Flag (C)
    cpsr |= (1 << 29);
  if (overflow)  // Overflow Flag (V)
    cpsr |= (1 << 28);
}

}  // namespace ARM