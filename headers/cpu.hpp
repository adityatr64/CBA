// Thank you to https://problemkaputt.de/gbatek.htm#gbamemorymap for the memory map
#pragma once
#include <cstdint>

// Screen dimensions
#define WIDTH 240
#define HEIGHT 160

struct Registers
{

  // make FIQ registers
  uint32_t r[16]; // 16 general-purpose registers (r0-r15)
  uint32_t cpsr;
  uint32_t &sp;
  uint32_t &lr;
  uint32_t &pc;
  Registers() : sp(r[13]), lr(r[14]), pc(r[15]) {}
};

/*  cpsr, sp, lr, pc: These are special registers:
    cpsr: The Current Program Status Register. It contains flags that control the CPU's state (e.g., N, Z, C, V flags).
    sp: The Stack Pointer. It points to the top of the stack in memory.
    lr: The Link Register. It stores the return address when a function is called.
    pc: The Program Counter. It stores the address of the next inst to be executed.
*/

class Memory; // Forward declaration

class CPU
{
private:
  Registers registers; // Instance of the Registers struct
  Memory &memory;      // Reference to your Memory object (you'll need this)

public:
  CPU(Memory &mem); // Constructor takes a reference to memory

  uint32_t readRegister(int index) const;
  void writeRegister(int index, uint32_t value);
  void executeinst();

  void run();
  void decodeThumb(uint16_t inst);
  void detectThumbinst();

  bool checkCondition(uint32_t inst);
  void decodeARM(uint32_t inst);

  void executeArmLoadStore(uint32_t inst);
  void executeArmBranch(uint32_t inst);
  void executeArmUndefined(uint32_t inst);
  void executeArmALU(uint32_t inst);
  void executeArmBlockTransfer(uint32_t inst);
  void executeArmBranchLink(uint32_t inst);
  void executeArmSoftwareInterrupt(uint32_t inst);
  void executeArmSWP(uint32_t inst);

  void updateFlags(uint32_t result, bool carry, bool overflow);
  uint32_t selectOperand2(uint32_t inst, bool &carryOut);
  uint32_t Shifter(uint32_t value, uint32_t type, uint32_t amount, bool &carryout);
};