#pragma once
#include <cstdint>

// Forward declarations
class CPU;
class Memory;

// Bitfield extraction helper macros
#define EXTRACT_BITS(value, start, count) (((value) >> (start)) & ((1 << (count)) - 1))
#define CHECK_BIT(value, bit) (((value) >> (bit)) & 1)

namespace ARM {
typedef void (*InstructionHandler)(CPU* cpu, Memory* memory, uint32_t inst);

struct InstructionEntry {
  uint32_t mask;    
  uint32_t pattern;  
  InstructionHandler handler;
  const char* name;
};

// ARM instruction decoding and execution
void decodeARM(CPU* cpu, Memory* memory, uint32_t inst);
bool checkCondition(CPU* cpu, uint32_t inst);

// ARM instruction execution functions
void executeArmLoadStore(CPU* cpu, Memory* memory, uint32_t inst);
void executeArmBranch(CPU* cpu, uint32_t inst);
void executeArmUndefined(uint32_t inst);
void executeArmALU(CPU* cpu, uint32_t inst);
void executeArmBlockTransfer(CPU* cpu, Memory* memory, uint32_t inst);
void executeArmBranchLink(CPU* cpu, uint32_t inst);
void executeArmSoftwareInterrupt(uint32_t inst);
void executeArmSWP(CPU* cpu, Memory* memory, uint32_t inst);
void executeArmMultiply(CPU* cpu, uint32_t inst);

// wrappers
void wrappedExecuteArmBranch(CPU* cpu, Memory* memory, uint32_t inst);
void wrappedExecuteArmMultiply(CPU* cpu, Memory* memory, uint32_t inst);
void wrappedExecuteArmSWI(CPU* cpu, Memory* memory, uint32_t inst);
void wrappedExecuteArmUndefined(CPU* cpu, Memory* memory, uint32_t inst);
void wrappedExecuteArmBranchLink(CPU* cpu, Memory* memory, uint32_t inst);
void wrappedExecuteArmALU(CPU* cpu, Memory* memory, uint32_t inst);

// ARM helper functions
void updateFlags(CPU* cpu, uint32_t result, bool carry, bool overflow);
uint32_t selectOperand2(CPU* cpu, uint32_t inst, bool& carryOut);
uint32_t Shifter(CPU* cpu, uint32_t value, uint32_t type, uint32_t amount, bool& carryout);
}  // namespace ARM