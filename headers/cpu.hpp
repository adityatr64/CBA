// Thank you to https://problemkaputt.de/gbatek.htm#gbamemorymap for the memory map
#pragma once
#include <cstdint>

struct Registers {
  uint32_t r[16];  // 16 general-purpose registers (r0-r15)
  uint32_t cpsr;   // Current Program Status Register
  uint32_t &sp;    // Stack Pointer (r13)
  uint32_t &lr;    // Link Register (r14)
  uint32_t &pc;    // Program Counter (r15)
  Registers() : sp(r[13]), lr(r[14]), pc(r[15]) {}
};

class Memory;  // Forward declaration

class CPU {
 private:
  Registers registers;
  Memory &memory;

 public:
  CPU(Memory &mem);

  // === Core CPU Interface ===
  uint32_t readRegister(int index) const;
  void writeRegister(int index, uint32_t value);
  void executeinst();
  void run();

  // === Register Access for External Modules ===
  Registers &getRegisters() {
    return registers;
  }
  const Registers &getRegisters() const {
    return registers;
  }
  Memory &getMemory() {
    return memory;
  }

  // === Instruction Set Support ===
  // Thumb (16-bit) instructions
  void decodeThumb(uint16_t inst);
  void detectThumbinst();

  // === Utility Functions ===
  void updateFlags(uint32_t result, bool carry, bool overflow);
};