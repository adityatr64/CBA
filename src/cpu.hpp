// Thank you to https://problemkaputt.de/gbatek.htm#gbamemorymap for the memory map
#pragma once
#include <cstdint>

// Screen dimensions
#define WIDTH 240
#define HEIGHT 160

// Memory sizes
#define BIOS_SIZE 16 * 1024
#define WRAM_SIZE 256 * 1024
#define IWRAM_SIZE 32 * 1024
#define VRAM_SIZE 96 * 1024
#define OAM_SIZE 1 * 1024
#define PALETTE_SIZE 1 * 1024

#define BIOS_START 0x0000
#define BIOS_END 0x3FFF

// On-board RAM
#define WRAM_START 0x02000000
#define WRAM_END 0x0203FFFF

// On-chip RAM
#define IWRAM_START 0x03000000
#define IWRAM_END 0x03007FFF

// I/O Registers
#define IO_START 0x04000000
#define IO_END 0x040003FE

// Palette RAM
#define PALETTE_START 0x05000000
#define PALETTE_END 0x050003FF

// VRAM
#define VRAM_START 0x06000000
#define VRAM_END 0x06017FFF

// OAM
#define OAM_START 0x07000000
#define OAM_END 0x070003FF

// Game Pak ROM/FlashROM
#define ROM_START 0x08000000
#define ROM_END 0x09FFFFFF

// Game Pak SRAM
#define SRAM_START 0x0E000000
#define SRAM_END 0x0E00FFFF

struct Registers
{
  uint32_t r[16]; // 16 general-purpose registers (r0-r15)
  uint32_t cpsr;  // Current Program Status Register
  uint32_t &sp;   // Stack Pointer (r13)
  uint32_t &lr;   // Link Register (r14)
  uint32_t &pc;   // Program Counter (r15)

  Registers() : sp(r[13]), lr(r[14]), pc(r[15]) {}
};
;
/*  cpsr, sp, lr, pc: These are special registers:
    cpsr: The Current Program Status Register. It contains flags that control the CPU's state (e.g., N, Z, C, V flags).
    sp: The Stack Pointer. It points to the top of the stack in memory.
    lr: The Link Register. It stores the return address when a function is called.
    pc: The Program Counter. It stores the address of the next instruction to be executed.
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

  void executeInstruction();

  void run();
};