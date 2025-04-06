#pragma once
#include <cstdint>
#include <string>
#include <vector>

class Memory {
 private:
  std::vector<uint8_t> bios;
  std::vector<uint8_t> wram;
  std::vector<uint8_t> iwram;
  std::vector<uint8_t> io;
  std::vector<uint8_t> palette;
  std::vector<uint8_t> vram;
  std::vector<uint8_t> oam;
  std::vector<uint8_t> rom;

 public:
  Memory();  // Constructor
  // For ARM mode
  uint32_t readWord(uint32_t address) const;
  void writeWord(uint32_t address, uint32_t value);

  // For Thumb mode/HalfWord arm instructions
  uint16_t readHalfWord(uint32_t address) const;
  void writeHalfWord(uint32_t address, uint16_t value);

  uint8_t readByte(uint32_t address) const;
  void writeByte(uint32_t address, uint8_t value);

  // For both ARM and Thumb modes
  uint32_t loadBinFile(const std::string &filename);
  size_t getROMSize() const;
  void dumpROM() const;
};

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